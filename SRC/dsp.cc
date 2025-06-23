/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *                2019  Eddie Hung    <eddie@fpgeh.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/yosys.h"
#include "kernel/sigtools.h"
#include <deque>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

#include "../ARCHITECTURE/Z1010/DSP/dsp_CREG_pm.h"
#include "../ARCHITECTURE/Z1010/DSP/dsp_cascade_pm.h"

void dsp_packC(dsp_CREG_pm &pm)
{
	auto &st = pm.st_dsp_packC;

	log_debug("Analyzing %s.%s for Yosys_syn_ Z1010 DSP packing (CREG).\n", log_id(pm.module), log_id(st.dsp));
	log_debug("ffC:        %s\n", log_id(st.ffC, "--"));

	Cell *cell = st.dsp;

	if (st.clock != SigBit())
	{
		cell->setPort(ID::CLK, st.clock);

		auto f = [&pm,cell](SigSpec &A, Cell* ff, IdString ceport, IdString rstport) {
			SigSpec D = ff->getPort(ID::D);
			SigSpec Q = pm.sigmap(ff->getPort(ID::Q));
			if (!A.empty())
				A.replace(Q, D);
			if (rstport != IdString()) {
				if (ff->type.in(ID($sdff), ID($sdffe))) {
					SigSpec srst = ff->getPort(ID::SRST);
					bool rstpol = ff->getParam(ID::SRST_POLARITY).as_bool();
					cell->setPort(rstport, rstpol ? srst : pm.module->Not(NEW_ID, srst));
				} else {
					cell->setPort(rstport, State::S0);
				}
			}
			if (ff->type.in(ID($dffe), ID($sdffe))) {
				SigSpec ce = ff->getPort(ID::EN);
				bool cepol = ff->getParam(ID::EN_POLARITY).as_bool();
				cell->setPort(ceport, cepol ? ce : pm.module->Not(NEW_ID, ce));
			}
			else
				cell->setPort(ceport, State::S1);

			for (auto c : Q.chunks()) {
				auto it = c.wire->attributes.find(ID::init);
				if (it == c.wire->attributes.end())
					continue;
				for (int i = c.offset; i < c.offset+c.width; i++) {
					log_assert(it->second[i] == State::S0 || it->second[i] == State::Sx);
					it->second.bits()[i] = State::Sx;
				}
			}
		};

		if (st.ffC) {
			SigSpec C = cell->getPort(ID::C);
			f(C, st.ffC, ID(CEC), ID(RSTC));
			pm.add_siguser(C, cell);
			cell->setPort(ID::C, C);
			cell->setParam(ID(CREG), 1);
		}

		log("  clock: %s (%s)", log_signal(st.clock), "posedge");

		if (st.ffC)
			log(" ffC:%s", log_id(st.ffC));
		log("\n");
	}

	pm.blacklist(cell);
}

struct Yosys_syn_DspPass : public Pass {
	Yosys_syn_DspPass() : Pass("dsp", "Yosys_syn_: pack resources into DSPs") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    dsp [options] [selection]\n");
		log("\n");
		log("Pack input registers (A2, A1, B2, B1, C, D, AD; with optional enable/reset),\n");
		log("pipeline registers (M; with optional enable/reset), output registers (P; with\n");
		log("optional enable/reset), pre-adder and/or post-adder into Yosys_syn_ DSP resources.\n");
		log("\n");
		log("Multiply-accumulate operations using the post-adder with feedback on the 'C'\n");
		log("input will be folded into the DSP. In this scenario only, the 'C' input can be\n");
		log("used to override the current accumulation result with a new value, which will\n");
		log("be added to the multiplier result to form the next accumulation result.\n");
		log("\n");
		log("Use of the dedicated 'PCOUT' -> 'PCIN' cascade path is detected for 'P' -> 'C'\n");
		log("connections (optionally, where 'P' is right-shifted by 17-bits and used as an\n");
		log("input to the post-adder -- a pattern common for summing partial products to\n");
		log("implement wide multipliers). Limited support also exists for similar cascading\n");
		log("for A and B using '[AB]COUT' -> '[AB]CIN'. Currently, cascade chains are limited\n");
		log("to a maximum length of 20 cells, corresponding to the smallest 7 Series\n");
		log("device.\n");
		log("\n");
		log("This pass is a no-op if the scratchpad variable 'dsp.multonly' is set\n");
		log("to 1.\n");
		log("\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		log_header(design, "Executing DSP Inference pass (pack resources into DSPs).\n");

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++)
		{
		}
		extra_args(args, argidx, design);

		for (auto module : design->selected_modules()) {

			if (design->scratchpad_get_bool("dsp.multonly"))
				continue;

			// Separating out CREG packing is necessary since there
			//   is no guarantee that the cell ordering corresponds
			//   to the "expected" case (i.e. the order in which
			//   they appear in the source) thus the possiblity
			//   existed that a register got packed as a CREG into a
			//   downstream DSP that should have otherwise been a
			//   PREG of an upstream DSP that had not been visited
			//   yet
			{
				dsp_CREG_pm pm(module, module->selected_cells());
				pm.run_dsp_packC(dsp_packC);
			}
			// Lastly, identify and utilise PCOUT -> PCIN,
			//   ACOUT -> ACIN, and BCOUT-> BCIN dedicated cascade
			//   chains
			{
				dsp_cascade_pm pm(module, module->selected_cells());
				pm.run_dsp_cascade();
			}
		}
	}
} Yosys_syn_DspPass;

PRIVATE_NAMESPACE_END
