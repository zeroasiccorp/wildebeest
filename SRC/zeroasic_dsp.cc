/*
ISC License

Copyright (C) 2024 ZeroAsic Technology Inc. and its subsidiaries

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

#if 0
#include "techlibs/zeroasic/zeroasic_dsp_CREG_pm.h"
#include "techlibs/zeroasic/zeroasic_dsp_cascade_pm.h"
#include "techlibs/zeroasic/zeroasic_dsp_pm.h"
#endif

#include "zeroasic_dsp.h"

void zeroasic_dsp_pack(zeroasic_dsp_pm &pm)
{
	auto &st = pm.st_zeroasic_dsp_pack;

	log("Analysing %s.%s for ZeroAsic MAE packing.\n", log_id(pm.module), log_id(st.dsp));

	Cell *cell = st.dsp;
	// pack post-adder
	if (st.postAdderStatic) {
		// subtraction not yet handled
		
		cell->setParam(ID(POST_ADDER_STATIC), State::S1);
		if (st.useFeedBack) {
			cell->setParam(ID(USE_FEEDBACK), State::S1);
			cell->setPort(ID(CDIN_FDBK_SEL), {State::S0, State::S1});
		} else {
			cell->setParam(ID(USE_FEEDBACK), State::S0);
			// st.sigC.extend_u0(48, st.postAdderStatic->getParam(ID::A_SIGNED).as_bool());
			// cell->setPort(ID::C, st.sigC);
		}

		pm.autoremove(st.postAdderStatic);
	}

	if(st.multHasReg) {
		cell->setParam(ID(MULT_HAS_REG), State::S1);
	}
	else{
		cell->setParam(ID(MULT_HAS_REG), State::S0);

	}
	// pack registers
	if (st.clock != SigBit()) {
		cell->setPort(ID::CLK, st.clock);

		// function to absorb a register
		auto f = [&pm, cell](SigSpec &A, Cell *ff, IdString ceport, IdString rstport, IdString bypass, IdString bypass_param) {
			// input/output ports
			SigSpec D = ff->getPort(ID::D);
			SigSpec Q = pm.sigmap(ff->getPort(ID::Q));

			if (!A.empty())
				A.replace(Q, D);
			if (rstport != IdString()) {
				if (ff->type.in(ID($sdff), ID($sdffe))) {
					SigSpec srst = ff->getPort(ID::SRST);
					bool rstpol_n = !ff->getParam(ID::SRST_POLARITY).as_bool();
					// active low sync rst
					cell->setPort(rstport, rstpol_n ? srst : pm.module->Not(NEW_ID, srst));
				} else if (ff->type.in(ID($adff), ID($adffe))) {
					SigSpec arst = ff->getPort(ID::ARST);
					bool rstpol_n = !ff->getParam(ID::ARST_POLARITY).as_bool();
					// active low async rst
					cell->setPort(rstport, rstpol_n ? arst : pm.module->Not(NEW_ID, arst));
				} else {
					// active low async/sync rst
					cell->setPort(rstport, State::S1);
				}
			}
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
			cell->setPort(bypass, State::S0);
			cell->setParam(bypass_param, State::S1);

			for (auto c : Q.chunks()) {
				auto it = c.wire->attributes.find(ID::init);
				if (it == c.wire->attributes.end())
					continue;
				for (int i = c.offset; i < c.offset + c.width; i++) {
					log_assert(it->second[i] == State::S0 || it->second[i] == State::Sx);
					it->second.bits()[i] = State::Sx;
				}
			}
		};

		// NOTE: flops are not autoremoved because it is possible that they
		//       are only partially absorbed into DSP, or have fanouts.
		if (st.ffA) {
			SigSpec A = cell->getPort(ID::A);
			if (st.ffA) {
				f(A, st.ffA, ID(A_EN), ID(A_SRST_N), ID(A_BYPASS), ID(BYPASS_A));
			}
			pm.add_siguser(A, cell);
			cell->setPort(ID::A, A);
		}
		if (st.ffB) {
			SigSpec B = cell->getPort(ID::B);
			if (st.ffB) {
				f(B, st.ffB, ID(B_EN), ID(B_SRST_N), ID(B_BYPASS), ID(BYPASS_B));
			}
			pm.add_siguser(B, cell);
			cell->setPort(ID::B, B);
		}
		if (st.ffC) {
			SigSpec C = cell->getPort(ID::C);
			if (st.ffC->type.in(ID($adff), ID($adffe))) {
				f(C, st.ffC, ID(C_EN), ID(C_ARST_N), ID(C_BYPASS), ID(BYPASS_C));
			} else {
				f(C, st.ffC, ID(C_EN), ID(C_SRST_N), ID(C_BYPASS), ID(BYPASS_C));
			}

			pm.add_siguser(C, cell);
			cell->setPort(ID::C, C);
		}
		if (st.ffP){
			SigSpec P; // unused
			f(P, st.ffP, ID(P_EN), ID(P_SRST_N), ID(P_BYPASS), ID(BYPASS_P));
			st.ffP->connections_.at(ID::Q).replace(st.sigP, pm.module->addWire(NEW_ID, GetSize(st.sigP)));
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

struct ZeroAsicDspPass : public Pass {
	ZeroAsicDspPass() : Pass("zeroasic_dsp", "ZEROASIC: pack resources into DSPs") {}
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    zeroasic_dsp [options] [selection]\n");
		log("\n");
		log("Pack input registers 'A', 'B', 'C', and 'D' (with optional enable/reset),\n");
		log("output register 'P' (with optional enable/reset), pre-adder and/or post-adder into\n");
		log("ZeroAsic DSP resources.\n");
		log("\n");
		log("Multiply-accumulate operations using the post-adder with feedback on the 'C'\n");
		log("input will be folded into the DSP. In this scenario only, the 'C' input can be\n");
		log("used to override the current accumulation result with a new value. This will\n");
		log("be added to the multiplier result to form the next accumulation result.\n");
		log("\n");
		log("Use of the dedicated 'PCOUT' -> 'PCIN' cascade path is detected for 'P' -> 'C'\n");
		log("connections (optionally, where 'P' is right-shifted by 17-bits and used as an\n");
		log("input to the post-adder. This pattern is common for summing partial products to\n");
		log("implement wide multipliers). Cascade chains are limited to a mazimum length \n");
		log("of 24 cells, corresponding to PolarFire (pf) devices.\n");
		log("\n");
		log("This pass is a no-op if the scratchpad variable 'zeroasic_dsp.multonly' is set\n");
		log("to 1.\n");
		log("\n");
		log("\n");
		log("    -family {polarfire}\n");
		log("        select the family to target\n");
		log("        default: polarfire\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		log_header(design, "Executing ZEROASIC_DSP pass (pack resources into DSPs).\n");

		std::string family = "polarfire";
		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++) {
			if ((args[argidx] == "-family") && argidx + 1 < args.size()) {
				family = args[++argidx];
				continue;
			}
			break;
		}
		extra_args(args, argidx, design);

		for (auto module : design->selected_modules()) {

			if (design->scratchpad_get_bool("zeroasic_dsp.multonly"))
				continue;

			{
				// For more details on PolarFire MACC_PA, consult
				//   the "PolarFire FPGA Macro Library Guide"

				// Main pattern matching step to capture a DSP cell.
				//   Match for pre-adder, post-adder, as well as
				//   registers 'A', 'B', 'D', and 'P'. Additionally,
				//   check for an accumulator pattern based on whether
				//   a post-adder and PREG are both present AND
				//   if PREG feeds into this post-adder.
				zeroasic_dsp_pm pm(module, module->selected_cells());
				pm.run_zeroasic_dsp_pack(zeroasic_dsp_pack);
			}
		}		
	}
} ZeroAsicDspPass;

PRIVATE_NAMESPACE_END
