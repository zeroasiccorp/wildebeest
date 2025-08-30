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
#include "techlibs/zeroasic/zeroasic_dsp_noff_CREG_pm.h"
#include "techlibs/zeroasic/zeroasic_dsp_noff_cascade_pm.h"
#include "techlibs/zeroasic/zeroasic_dsp_noff_pm.h"
#endif

#include "zeroasic_dsp_noff.h"

void zeroasic_dsp_noff_pack(zeroasic_dsp_noff_pm &pm)
{
	auto &st = pm.st_zeroasic_dsp_noff_pack;

	log("Analysing %s.%s for ZeroAsic MACC_PA packing.\n", log_id(pm.module), log_id(st.dsp));

	Cell *cell = st.dsp;
	// pack post-adder
	if (st.postAdderStatic) {
		log("  postadder %s (%s)\n", log_id(st.postAdderStatic), log_id(st.postAdderStatic->type));
		SigSpec &sub = cell->connections_.at(ID(SUB));
		// Post-adder in MACC_PA also supports subtraction
		//   Determines the sign of the output from the multiplier.
		if (st.postAdderStatic->type == ID($add))
			sub[0] = State::S0;
		else if (st.postAdderStatic->type == ID($sub))
			sub[0] = State::S1;
		else
			log_assert(!"strange post-adder type");

		if (st.useFeedBack) {
			cell->setPort(ID(CDIN_FDBK_SEL), {State::S0, State::S1});
		} else {
			st.sigC.extend_u0(48, st.postAdderStatic->getParam(ID::A_SIGNED).as_bool());
			cell->setPort(ID::C, st.sigC);
		}

		pm.autoremove(st.postAdderStatic);
	}

	log("\n");

	SigSpec P = st.sigP;
	if (GetSize(P) < 48)
		P.append(pm.module->addWire(NEW_ID, 48 - GetSize(P)));
	cell->setPort(ID::P, P);

	pm.blacklist(cell);
}

struct ZeroAsicDspNoFfPass : public Pass {
	ZeroAsicDspNoFfPass() : Pass("zeroasic_dsp_noff", "ZEROASIC: pack resources into DSPs") {}
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    zeroasic_dsp_noff [options] [selection]\n");
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
		log("This pass is a no-op if the scratchpad variable 'zeroasic_dsp_noff.multonly' is set\n");
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

			if (design->scratchpad_get_bool("zeroasic_dsp_noff.multonly"))
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
				zeroasic_dsp_noff_pm pm(module, module->selected_cells());
				pm.run_zeroasic_dsp_noff_pack(zeroasic_dsp_noff_pack);
			}

		}
	}
} ZeroAsicDspNoFfPass;

PRIVATE_NAMESPACE_END
