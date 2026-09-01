/*
ISC License

Copyright (C) 2025  Frederick Tombs <fred@zeroasic.com>, Zero Asic Corp.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "kernel/sigtools.h"
#include "kernel/yosys.h"
#include <deque>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

#include "zeroasic_dsp.h"

// An 18 bit add costs about as many LUTs as it has result bits, so anything
// narrower than this is not worth a DSP that a multiplier could have used.
static const int DEFAULT_ADD_MINWIDTH = 10;

void zeroasic_dsp_pack(zeroasic_dsp_pm &pm) {
  auto &st = pm.st_zeroasic_dsp_pack;

  Cell *cell = st.dsp;
  // pack post-adder
  //
  if (st.postAdderStatic) {
    cell->setParam(ID(POST_ADDER_STATIC), State::S1);
    if (st.useFeedBack) {
      cell->setParam(ID(USE_FEEDBACK), State::S1);
      cell->setPort(ID(CDIN_FDBK_SEL), {State::S0, State::S1});
    } else {
      cell->setParam(ID(USE_FEEDBACK), State::S0);
      cell->setPort(ID::C, st.sigC);
    }

    pm.autoremove(st.postAdderStatic);
  }

  if (st.multHasReg) {
    cell->setParam(ID(MULT_HAS_REG), State::S1);
    pm.autoremove(st.ffMult);
  } else {
    cell->setParam(ID(MULT_HAS_REG), State::S0);
  }
  // pack registers
  //
  if (st.clock != SigBit()) {
    cell->setPort(ID::CLK, st.clock);

    // function to absorb a register
    auto f = [&pm, cell](SigSpec &A, Cell *ff, IdString ceport,
                         IdString rstport, IdString bypass,
                         IdString bypass_param) {
      // input/output ports
      SigSpec D = ff->getPort(ID::D);
      SigSpec Q = pm.sigmap(ff->getPort(ID::Q));

      if (!A.empty())
        A.replace(Q, D);

      if (ff->type.in(ID($dffe), ID($sdffe), ID($adffe))) {
        SigSpec ce = ff->getPort(ID::EN);
        bool cepol = ff->getParam(ID::EN_POLARITY).as_bool();
        // enables are all active high
        cell->setPort(ceport, cepol ? ce : pm.module->Not(NEW_ID, ce));
      } else {
        // enables are all active high
        cell->setPort(ceport, State::S1);
      }

      // bypass set to 0
      cell->setParam(bypass_param, State::S1);

      for (auto c : Q.chunks()) {
        auto it = c.wire->attributes.find(ID::init);
        if (it == c.wire->attributes.end())
          continue;
        for (int i = c.offset; i < c.offset + c.width; i++) {
          log_assert(it->second[i] == State::S0 || it->second[i] == State::Sx);
#if YOSYS_MAJOR == 0 && YOSYS_MINOR <= 57
          it->second.bits()[i] = State::Sx;
#else
	  it->second.set(i, State::Sx);
#endif
        }
      }
    };

    if (st.ffA && st.ffB) { // both A and B have to be registered
      SigSpec A = cell->getPort(ID::A);
      if (st.ffA) {
        f(A, st.ffA, ID(A_EN), ID(A_ARST_N), ID(ALLOW_A_REG), ID(A_REG));
      }
      pm.add_siguser(A, cell);
      cell->setPort(ID::A, A);

      SigSpec B = cell->getPort(ID::B);
      if (st.ffB) {
        f(B, st.ffB, ID(B_EN), ID(B_ARST_N), ID(ALLOW_B_REG), ID(B_REG));
      }
      pm.add_siguser(B, cell);
      cell->setPort(ID::B, B);

      // set the reset port to the reset (which has to be shared by both A and
      // B)
      if (st.ffA->type.in(ID($sdff), ID($sdffe))) {
        log("Error: synchronous DSPs not packable on MAE\n");
        log_assert(not st.ffA->type.in(ID($sdff), ID($sdffe)));
      } else if (st.ffA->type.in(ID($adff), ID($adffe))) {
        SigSpec arst = st.ffA->getPort(ID::ARST);
        bool rstpol_n = !st.ffA->getParam(ID::ARST_POLARITY).as_bool();
        // active low async rst
        cell->setPort(ID(resetn),
                      rstpol_n ? arst : pm.module->Not(NEW_ID, arst));
      } else {
        // active low async/sync rst
        cell->setPort(ID(resetn), State::S1);
      }
    } else {
      cell->setParam(ID(A_REG), State::S0);
      cell->setParam(ID(B_REG), State::S0);
    }

    if (st.ffC) {
      SigSpec C = cell->getPort(ID::C);
      f(C, st.ffC, ID(C_EN), ID(C_ARST_N), ID(ALLOW_C_REG), ID(C_REG));

      pm.add_siguser(C, cell);
      cell->setPort(ID::C, C);
    }
    if (st.ffP) {
      SigSpec P; // unused
      f(P, st.ffP, ID(P_EN), ID(P_ARST_N), ID(ALLOW_P_REG), ID(P_REG));
      st.ffP->connections_.at(ID::Q).replace(
          st.sigP, pm.module->addWire(NEW_ID, GetSize(st.sigP)));

      // Set resetn from ffP when A/B registers were not packed (the A/B block
      // above only runs when ffA && ffB, so resetn would otherwise be undriven
      // for the output-register-only case, e.g. efpga_mult_rego).
      //
      // The test is on the whole A/B condition and not on ffA alone.  The .pmg
      // matches ffA and ffB independently, so ffA can be matched with ffB not
      // -- a multiplier with one registered operand and one combinational one,
      // which is what a pre-adder feeding a multiplier looks like -- and then
      // neither block assigned resetn.  The cell kept the unconnected
      // MAE.resetn port, VPR's Netlist::compress() indexed pin_net_indices_ at
      // -1 on the driverless net, and the run died as 'free(): invalid pointer'
      // with nothing naming the cell.
      if (!(st.ffA && st.ffB)) {
        if (st.ffP->type.in(ID($adff), ID($adffe))) {
          SigSpec arst = st.ffP->getPort(ID::ARST);
          bool rstpol_n = !st.ffP->getParam(ID::ARST_POLARITY).as_bool();
          cell->setPort(ID(resetn),
                        rstpol_n ? arst : pm.module->Not(NEW_ID, arst));
        } else {
          cell->setPort(ID(resetn), State::S1);
        }
      }
    }

    log("  clock: %s (%s)\n", log_signal(st.clock), "posedge");

    if (st.ffA)
      log(" \t ffA:%s\n", log_id(st.ffA));
    if (st.ffB)
      log(" \t ffB:%s\n", log_id(st.ffB));
    if (st.ffC)
      log(" \t ffC:%s\n", log_id(st.ffC));
    if (st.ffP)
      log(" \t ffP:%s\n", log_id(st.ffP));
  }
  log("\n");

  SigSpec P = st.sigP;
  if (GetSize(P) < 40)
    P.append(pm.module->addWire(NEW_ID, 40 - GetSize(P)));
  cell->setPort(ID::P, P);

  pm.blacklist(cell);
}

// The six add-only MAE modes ('efpga_adder', 'efpga_adder_regi',
// 'efpga_adder_rego', 'efpga_adder_regio', 'efpga_acc' and 'efpga_acc_regi')
// have no multiplier in them, so the '$mul' driven 'mul2dsp' techmap never
// produces a MAE for a design that only adds or accumulates. Seed one here
// from the '$add' that 'zeroasic_dsp_add_pack' matched, with 'ADD_ONLY' set so
// that 'zeroasic_dsp_map_mode.v' stays the single place where a mode is picked.
//
void zeroasic_dsp_add_pack(zeroasic_dsp_pm &pm) {
  auto &st = pm.st_zeroasic_dsp_add_pack;

  Module *module = pm.module;
  Cell *add = st.add;

  Cell *ffA = st.ffA;
  Cell *ffB = st.ffB;
  Cell *ffP = st.ffP;

  // 'efpga_adder_regi' and 'efpga_adder_regio' register both operands out of
  // the same flops, so a single registered operand is not a mode we have.
  if (!st.useFeedBack && !(ffA && ffB)) {
    ffA = nullptr;
    ffB = nullptr;
  }

  // The macro has one active low asynchronous reset shared by every register
  // it holds, so only pack flops that agree on it. '$dff' (no reset at all)
  // does not agree with a flop that has one.
  auto arst = [](Cell *ff) {
    if (ff && ff->type.in(ID($adff), ID($adffe)))
      return ff->getPort(ID::ARST);
    return SigSpec();
  };
  auto arstPolarity = [](Cell *ff) {
    if (ff && ff->type.in(ID($adff), ID($adffe)))
      return ff->getParam(ID::ARST_POLARITY).as_bool();
    return false;
  };
  auto sameReset = [&](Cell *a, Cell *b) {
    return arst(a) == arst(b) && arstPolarity(a) == arstPolarity(b);
  };

  if (ffP && ffA && !sameReset(ffP, ffA)) {
    ffA = nullptr;
    ffB = nullptr;
  }
  if (ffA && ffB && !sameReset(ffA, ffB)) {
    ffA = nullptr;
    ffB = nullptr;
  }

  // Move an operand past the register that feeds it, and extend it to the
  // 18 bits of the macro input port.
  auto operand = [&](SigSpec sig, Cell *ff, bool is_signed) {
    if (ff) {
      SigSpec D = ff->getPort(ID::D);
      SigSpec Q = pm.sigmap(ff->getPort(ID::Q));
      sig.replace(Q, D);
    }
    SigBit pad = is_signed ? sig[GetSize(sig) - 1] : SigBit(State::S0);
    while (GetSize(sig) < 18)
      sig.append(pad);
    return sig;
  };

  Cell *cell = module->addCell(NEW_ID, ID(MAE));

  cell->setParam(ID(ADD_ONLY), State::S1);
  cell->setParam(ID(POST_ADDER_STATIC), State::S0);
  cell->setParam(ID(MULT_HAS_REG), State::S0);
  cell->setParam(ID(C_REG), State::S0);
  cell->setParam(ID(USE_FEEDBACK), st.useFeedBack ? State::S1 : State::S0);
  cell->setParam(ID(A_REG), ffA ? State::S1 : State::S0);
  cell->setParam(ID(B_REG), ffB ? State::S1 : State::S0);
  cell->setParam(ID(P_REG), ffP ? State::S1 : State::S0);

  SigSpec A = operand(st.sigA, ffA, st.signedA);
  cell->setPort(ID::A, A);
  pm.add_siguser(A, cell);

  // An accumulator has no second operand: the other side of its adder is the
  // accumulator itself, which the macro feeds back internally.
  SigSpec B = st.useFeedBack ? SigSpec(State::S0, 18)
                             : operand(st.sigB, ffB, st.signedB);
  cell->setPort(ID::B, B);
  pm.add_siguser(B, cell);

  cell->setPort(ID::C, SigSpec(State::S0, 40));

  SigSpec P = st.sigP;
  if (GetSize(P) < 40)
    P.append(module->addWire(NEW_ID, 40 - GetSize(P)));
  cell->setPort(ID::P, P);

  if (st.clock != SigBit()) {
    cell->setPort(ID::CLK, st.clock);

    Cell *rstFrom = ffP ? ffP : ffA;
    SigSpec resetn = arst(rstFrom);
    if (resetn.empty())
      resetn = State::S1;
    else if (arstPolarity(rstFrom))
      resetn = module->Not(NEW_ID, resetn);
    cell->setPort(ID(resetn), resetn);

    log("  clock: %s (%s)\n", log_signal(st.clock), "posedge");
    if (ffA)
      log(" \t ffA:%s\n", log_id(ffA));
    if (ffB)
      log(" \t ffB:%s\n", log_id(ffB));
    if (ffP)
      log(" \t ffP:%s\n", log_id(ffP));
  }

  // Hand the output register's own net over to 'P' and leave the flop
  // dangling for 'opt_clean', the way the multiplier path does.
  if (ffP) {
    ffP->connections_.at(ID::Q).replace(
        st.sigP, module->addWire(NEW_ID, GetSize(st.sigP)));

    SigSpec Q = pm.sigmap(st.sigP);
    for (auto c : Q.chunks()) {
      if (c.wire == nullptr)
        continue;
      auto it = c.wire->attributes.find(ID::init);
      if (it == c.wire->attributes.end())
        continue;
      for (int i = c.offset; i < c.offset + c.width; i++) {
        if (i >= GetSize(it->second))
          break;
#if YOSYS_MAJOR == 0 && YOSYS_MINOR <= 57
        it->second.bits()[i] = State::Sx;
#else
        it->second.set(i, State::Sx);
#endif
      }
    }
  }

  log("\n");

  pm.autoremove(add);
  pm.blacklist(cell);
}

struct ZeroAsicDspPass : public Pass {
  ZeroAsicDspPass()
      : Pass("zeroasic_dsp", "ZEROASIC: pack resources into DSPs") {}
  void help() override {
    //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
    log("\n");
    log("    zeroasic_dsp [options] [selection]\n");
    log("\n");
    log("Pack input registers 'A', 'B', 'C', and 'D' (with optional "
        "enable/reset),\n");
    log("output register 'P' (with optional enable/reset), pre-adder and/or "
        "post-adder into\n");
    log("ZeroAsic DSP resources.\n");
    log("\n");
    log("A second pass then infers the add-only DSP modes ('efpga_adder*' "
        "and\n");
    log("'efpga_acc*') from any '$add' left over, which the '$mul' driven "
        "techmap\n");
    log("cannot reach.\n");
    log("\n");
    log("    -no_add\n");
    log("        do not infer the add-only modes, leaving every remaining "
        "'$add' to\n");
    log("        the fabric.\n");
    log("\n");
    log("    -add_minwidth <n>\n");
    log("        smallest add result width worth a DSP (default %d). Below "
        "this an\n",
        DEFAULT_ADD_MINWIDTH);
    log("        '$add' stays in the fabric so that narrow carry chains do "
        "not use up\n");
    log("        the DSPs.\n");
    log("\n");
  }

  void execute(std::vector<std::string> args, RTLIL::Design *design) override {
    log_header(design, "Executing ZEROASIC_DSP pass (pack DFFs into DSPs).\n");

    bool no_add = false;
    int add_minwidth = DEFAULT_ADD_MINWIDTH;

    size_t argidx;
    for (argidx = 1; argidx < args.size(); argidx++) {
      if (args[argidx] == "-no_add") {
        no_add = true;
        continue;
      }
      if ((args[argidx] == "-add_minwidth") && (argidx + 1 < args.size())) {
        add_minwidth = max(2, atoi(args[++argidx].c_str()));
        continue;
      }
      break;
    }
    extra_args(args, argidx, design);

    for (auto module : design->selected_modules()) {

      if (design->scratchpad_get_bool("zeroasic_dsp.multonly"))
        continue;

      {
        zeroasic_dsp_pm pm(module, module->selected_cells());
        pm.run_zeroasic_dsp_pack(zeroasic_dsp_pack);
      }

      // Runs on a fresh matcher so that the '$add' cells the multiplier pass
      // absorbed above are gone before the add-only modes get a look at what
      // is left.
      if (!no_add) {
        zeroasic_dsp_pm pm(module, module->selected_cells());
        pm.ud_zeroasic_dsp_add_pack.addMinWidth = add_minwidth;
        pm.run_zeroasic_dsp_add_pack(zeroasic_dsp_add_pack);
      }
    }
  }
} ZeroAsicDspPass;

PRIVATE_NAMESPACE_END
