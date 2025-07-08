/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
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

#include "kernel/register.h"
#include "kernel/celltypes.h"
#include "kernel/rtlil.h"
#include "kernel/log.h"
#include <chrono>


USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct LoadModelsPass : public ScriptPass
{
  // Global data
  //

  // Methods
  //
  LoadModelsPass() : ScriptPass("load_models", "Load Zero  Asic RTL models") { }

  void help() override
  {
	//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
	log("\n");
	log("    load_models\n");
	log("\n");

	help_script();
	log("\n");
  }

  void clear_flags() override
  {
  }

  void execute(std::vector<std::string> args, RTLIL::Design *design) override
  {
	string run_from, run_to;
	clear_flags();

	size_t argidx;
	for (argidx = 1; argidx < args.size(); argidx++)
	{
	}
	extra_args(args, argidx, design);

	log_header(design, "Executing Zero Asic load models.\n");
	log_push();

	run_script(design, run_from, run_to);

	log_pop();
  }

  // ---------------------------------------------------------------------------
  // load_models 
  // ---------------------------------------------------------------------------
  //
  void script() override
  {
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dff.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffe.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffr.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffs.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffrs.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffer.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffes.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffers.v");

  } // end script()

} LoadModelsPass;

PRIVATE_NAMESPACE_END
