//
// Zero Asic Corp. Plug-in
//
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
#include "kernel/sigtools.h"
#include "kernel/log.h"
#include "version.h"
#include <chrono>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <unistd.h>

#define SYNTH_FPGA_VERSION "1.0-" YOSYS_SYN_REVISION

#define HUGE_NB_CELLS 5000000 // 5 Million cells
#define BIG_NB_CELLS 500000   // 500K cells
#define SMALL_NB_CELLS 250000 // 250K cells
#define TINY_NB_CELLS 50000    // 50K cells

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct SynthFpgaPass : public ScriptPass
{
  // Global data
  //
  RTLIL::Design *G_design = NULL; 
  string top_opt, verilog_file, part_name, opt;
  string abc_script_version;
  bool no_flatten, dff_enable, dff_async_set, dff_async_reset;
  bool obs_clean, wait, show_max_level, csv, insbuf, resynthesis, autoname;
  bool no_opt_sat_dff, show_config, stop_if_undriven_nets;
  bool no_xor_tree_process;
  bool no_opt_const_dff;
  string sc_syn_lut_size;
  string sc_syn_fsm_encoding;
  string config_file = "";
  bool config_file_success = false;
  bool no_dsp;
  bool no_bram;
  bool use_sdff;
  string dsp_tech;
  string bram_tech;

  pool<string> opt_options  = {"default", "fast", "area", "delay"};
  pool<string> partnames  = {"Z1000", "Z1010"};
  pool<string> dsp_arch  = {"config", "xilinx", "microchip", "bare_mult", "mae"};
  pool<string> bram_arch  = {"config", "zeroasic", "microchip"};

  // ----------------------------
  // Key 'yosys-syn' parameters
  //
  string                  ys_root_path = "";

  // DFFs
  //
  pool<string>            ys_dff_features;
  dict<string, string>    ys_dff_models;
  string                  ys_dff_techmap = ""; 

  // BRAMs
  //
  vector<string>          ys_brams_memory_libmap; 
  vector<string>          ys_brams_memory_libmap_parameters; 
  vector<string>          ys_brams_techmap; 

  // DSPs
  //
  string                  ys_dsps_techmap = "";
  dict<string, int>       ys_dsps_parameter_int;
  dict<string, string>    ys_dsps_parameter_string;
  string                  ys_dsps_pack_command = "";

  // Methods
  //
  SynthFpgaPass() : ScriptPass("synth_fpga", "Zero Asic FPGA synthesis flow") { }


  // -------------------------
  // Json reader
  // -------------------------
  // Json node that stores section corresponding to 'config' file for 
  // synthesis
  //
  struct JsonNode
  {
	char type; // S=String, N=Number, A=Array, D=Dict
	string data_string;
	int64_t data_number;
	vector<JsonNode*> data_array;
	dict<string, JsonNode*> data_dict;
	vector<string> data_dict_keys;

	JsonNode(std::istream &f, string& cf_file, int& line)
	{
		type = 0;
		data_number = 0;

		while (1)
		{
			int ch = f.get();

			if (ch == EOF)
				log_error("Unexpected EOF in JSON file.\n");

			if (ch == '\n')
				line++;
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
				continue;

			if (ch == '"')
			{
				type = 'S';

				while (1)
				{
					ch = f.get();

					if (ch == EOF)
						log_error("Unexpected EOF in JSON string.\n");

					if (ch == '"')
						break;

					if (ch == '\\') {
						ch = f.get();

						switch (ch) {
							case EOF: log_error("Unexpected EOF in JSON string.\n"); break;
							case '"':
							case '/':
							case '\\':           break;
							case 'b': ch = '\b'; break;
							case 'f': ch = '\f'; break;
							case 'n': ch = '\n'; break;
							case 'r': ch = '\r'; break;
							case 't': ch = '\t'; break;
							case 'u':
								int val = 0;
								for (int i = 0; i < 4; i++) {
									ch = f.get();
									val <<= 4;
									if (ch >= '0' && '9' >= ch) {
										val += ch - '0';
									} else if (ch >= 'A' && 'F' >= ch) {
										val += 10 + ch - 'A';
									} else if (ch >= 'a' && 'f' >= ch) {
										val += 10 + ch - 'a';
									} else
										log_error("Unexpected non-digit character in \\uXXXX sequence: %c at line %d.\n", ch, line);
								}
								if (val < 128)
									ch = val;
								else
									log_error("Unsupported \\uXXXX sequence in JSON string: %04X at line %d.\n", val, line);
								break;
						}
					}

					data_string += ch;
				}

				break;
			}

			if (('0' <= ch && ch <= '9') || ch == '-')
			{
				bool negative = false;
				type = 'N';
				if (ch == '-') {
					data_number = 0;
				       	negative = true;
				} else {
					data_number = ch - '0';
				}

				data_string += ch;

				while (1)
				{
					ch = f.get();

					if (ch == EOF)
						break;

					if (ch == '.')
						goto parse_real;

					if (ch < '0' || '9' < ch) {
						f.unget();
						break;
					}

					data_number = data_number*10 + (ch - '0');
					data_string += ch;
				}

				data_number = negative ? -data_number : data_number;
				data_string = "";
				break;

			parse_real:
				type = 'S';
				data_number = 0;
				data_string += ch;

				while (1)
				{
					ch = f.get();

					if (ch == EOF)
						break;

					if (ch < '0' || '9' < ch) {
						f.unget();
						break;
					}

					data_string += ch;
				}

				break;
			}

			if (ch == '[')
			{
				type = 'A';

				while (1)
				{
					ch = f.get();

					if (ch == EOF)
						log_error("Unexpected EOF in JSON file '%s'.\n",
							  cf_file.c_str());

			                if (ch == '\n')
				                line++;

					if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ',')
						continue;

					if (ch == ']')
						break;

					f.unget();
					data_array.push_back(new JsonNode(f, cf_file, line));
				}

				break;
			}

			if (ch == '{')
			{
				type = 'D';

				while (1)
				{
					ch = f.get();

					if (ch == EOF)
						log_error("Unexpected EOF in JSON file '%s'.\n",
							  cf_file.c_str());

			                if (ch == '\n')
				                line++;

					if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ',')
						continue;

					if (ch == '}')
						break;

					f.unget();
					JsonNode key(f, cf_file, line);

					while (1)
					{
						ch = f.get();

						if (ch == EOF)
						       log_error("Unexpected EOF in JSON file '%s'.\n",
							         cf_file.c_str());

			                        if (ch == '\n')
				                        line++;

						if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ':')
							continue;

						f.unget();
						break;
					}

					JsonNode *value = new JsonNode(f, cf_file, line);

					if (key.type != 'S')
						log_error("Unexpected non-string key in JSON dict at line %d.\n", line);

					data_dict[key.data_string] = value;
					data_dict_keys.push_back(key.data_string);
				}

				break;
			}

			log_error("Unexpected character '%c' in config file '%s' at line %d.\n", 
		                  ch, cf_file.c_str(), line);
		}
	}

	~JsonNode()
	{
		for (auto it : data_array)
			delete it;
		for (auto &it : data_dict)
			delete it.second;
	}
  };
  
  // -------------------------
  // Example of config file 
  // -------------------------
#if 0
  {
  "version": 3,
  "partname": "Z1010",
  "lut_size": 6,
  "root_path" : "/home/thierry/YOSYS_DYN/yosys/yosys-syn/",
  "flipflops": {
                "features": ["async_reset", "async_set", "flop_enable"]
                "models": {
                        "dffers": "SRC/FF_MODELS/dffers.v",
                        "dffer": "SRC/FF_MODELS/dffer.v",
                        "dffes": "SRC/FF_MODELS/dffes.v",
                        "dffe": "SRC/FF_MODELS/dffe.v",
                        "dffrs": "SRC/FF_MODELS/dffrs.v",
                        "dffr": "SRC/FF_MODELS/dffr.v",
                        "dffs": "SRC/FF_MODELS/dffs.v",
                        "dff": "SRC/FF_MODELS/dff.v"
                },
                "techmap": "ARCHITECTURE/Z1010/techlib/tech_flops.v"
        },
  "brams": {
            "memory_libmap": [ "ARCHITECTURE/Z1010/BRAM/LSRAM.txt",
                               "ARCHITECTURE/Z1010/BRAM/uSRAM.txt"]
            "memory_libmap_parameters": ["-logic-cost-rom 0.5"]
            "techmap": ["ARCHITECTURE/Z1010/BRAM/LSRAM_map.v",
                        "ARCHITECTURE/Z1010/BRAM/uSRAM_map.v"]
        },
  "dsps": {
          "family": "microchip",
          "techmap": "../techlibs/microchip/polarfire_dsp_map.v",
          "techmap_parameters": {
                   "DSP_A_MAXWIDTH": 18,
                   "DSP_B_MAXWIDTH": 18,
                   "DSP_A_MAXWIDTH_PARTIAL": 18,
                   "DSP_A_MINWIDTH": 2,
                   "DSP_B_MINWIDTH": 2,
                   "DSP_Y_MINWIDTH": 9,
                   "DSP_SIGNEDONLY": 1,
                   "DSP_NAME": "$__MUL18X18"
          },
          "pack_command": "microchip_dsp -family polarfire"
       }
}
~
#endif

  // ----------------------------------------------
  // Structure to store all config file sections
  // ----------------------------------------------
  typedef struct {
	  string config_file;
	  int    version;
	  string root_path;
	  string partname;
	  int    lut_size;

	  // DFF related
	  //
	  pool<string>         dff_features;
	  dict<string, string> dff_models;
	  string               dff_techmap;

          // BRAM related
	  //
	  vector<string>       brams_memory_libmap;
	  vector<string>       brams_memory_libmap_parameters;
	  vector<string>       brams_techmap;
	  
	  //
          // DSP related
	  //
	  string               dsps_family;
	  string               dsps_techmap;
	  dict<string, int>    dsps_parameter_int;
	  dict<string, string> dsps_parameter_string;
	  string               dsps_pack_command;

  } config_type;

  // The global config object
  //
  config_type G_config;

  // -------------------------
  // show_config_file
  // -------------------------
  void show_config_file() 
  {

    if (!show_config) {
      return;
    }

    log_header(G_design, "Show config file : \n");

    log("\n");
    log(" ==========================================================================\n");
    log("  Config file        : %s\n", (G_config.config_file).c_str());
    log("  version            : %d\n", G_config.version);
    log("  partname           : %s\n", (G_config.partname).c_str());
    log("  lut_size           : %d\n", G_config.lut_size);
    log("  root_path          : %s\n", (G_config.root_path).c_str());

    log("  DFF Features       : \n");
    for (auto it : G_config.dff_features) {
       log("                       %s\n", it.c_str());
    }

    log("  DFF MODELS         : \n");
    for (auto it : G_config.dff_models) {
       log("                       %s %s\n", (it.first).c_str(), (it.second).c_str());
    }

    log("  DFF techmap        : \n");
    log("                       %s\n", (G_config.dff_techmap).c_str());

    log("  BRAM memory_libmap : \n");
    for (auto it : G_config.brams_memory_libmap) {
       log("                       %s\n", it.c_str());
    }
    log("\n");
    log("  BRAM memory_libmap parameters : \n");
    for (auto it : G_config.brams_memory_libmap_parameters) {
       log("                       %s\n", it.c_str());
    }

    log("  BRAM techmap       : \n");
    for (auto it : G_config.brams_techmap) {
       log("                       %s\n", it.c_str());
    }

    log("  DSP family         : \n");
    log("                       %s\n", (G_config.dsps_family).c_str());

    log("  DSP techmap        : \n");
    log("                       %s\n", (G_config.dsps_techmap).c_str());

    log("  DSP techparam      : \n");

    log("      int param      : \n");
    for (auto it : G_config.dsps_parameter_int) {
       log("                       %s = %d\n", (it.first).c_str(), it.second);
    }

    log("      string param   : \n");
    for (auto it : G_config.dsps_parameter_string) {
       log("                       %s = %s\n", (it.first).c_str(), (it.second).c_str());
    }
    log("\n");
    log("  DSP pack_command   : \n");
    log("                       %s\n", (G_config.dsps_pack_command).c_str());

    log(" ==========================================================================\n");

    // Wait a bit to see the config file data on the screen
    //
    usleep(5000000);
  }

  // -------------------------
  // setup_options
  // -------------------------
  // We setup the options according to 'config' file, if any, and command
  // line options.
  // If options conflict (ex: lut_size) between the two, 'config' file 
  // setting has the priority.
  //
  void setup_options()
  {
    // If there is a config file with successful analysis then we set up
    // all the yosys-syn parameters with it.
    //
    if (config_file_success) {

       ys_root_path = G_config.root_path;


       // DFF parameters setting
       //
       for (auto it : G_config.dff_features) {
          ys_dff_features.insert(it);
       }
       for (auto it : G_config.dff_models) {
          ys_dff_models[it.first] = it.second;
       }
       ys_dff_techmap = G_config.root_path + "/" + G_config.dff_techmap;


       // BRAMs parameters setting
       //
       ys_brams_memory_libmap.clear();

       if (G_config.brams_memory_libmap.size() != 0) {

         for (auto it : G_config.brams_memory_libmap) {
             string path = G_config.root_path + "/" + it;
	     ys_brams_memory_libmap.push_back(path);
         }
       }

       ys_brams_memory_libmap_parameters.clear();

       if (G_config.brams_memory_libmap_parameters.size() != 0) {

         for (auto it : G_config.brams_memory_libmap_parameters) {
             string param = it;
	     ys_brams_memory_libmap_parameters.push_back(param);
         }
       }

       ys_brams_techmap.clear(); 

       if (G_config.brams_techmap.size() != 0) {

          for (auto it : G_config.brams_techmap) {
              string path = G_config.root_path + "/" + it;
	      ys_brams_techmap.push_back(path);
          }
       }
       bram_tech = "config";

       
       // DSPs parameters setting
       //
       if (G_config.dsps_techmap == "") {

          ys_dsps_techmap = "";

       } else {

          ys_dsps_techmap = G_config.root_path + "/" + G_config.dsps_techmap;

          for (auto it : G_config.dsps_parameter_int) {
	      ys_dsps_parameter_int[it.first] = it.second;
          }
          for (auto it : G_config.dsps_parameter_string) {
	      ys_dsps_parameter_string[it.first] = it.second;
          }

          ys_dsps_pack_command = G_config.dsps_pack_command;
       }


       // We call the 'dsp_tech' through the config file mechanism as 'config' (why not ? ;-)
       //
       dsp_tech = "config";


       // Processing cases where 'config' file overides user command options.
       //
       if (ys_dff_features.count("flop_enable") == 0) {
         log_warning("Config file will switch on '-no_dff_enable' option.\n");
         dff_enable = false;
       }

       if (ys_dff_features.count("async_reset") == 0) {
         log_warning("Config file will switch on '-no_dff_async_reset' option.\n");
         dff_async_reset = false;
       }

       if (ys_dff_features.count("async_set") == 0) {
         log_warning("Config file will switch on '-no_dff_async_set' option.\n");
         dff_async_set = false;
       }
       
       if (std::to_string(G_config.lut_size) != sc_syn_lut_size) {
         log_warning("Config file will change lut size value from %s to %d.\n",
	             sc_syn_lut_size.c_str(), G_config.lut_size);
         sc_syn_lut_size = std::to_string(G_config.lut_size);
       }

    } else {

      // Default settings when 'config' file is not specified.
    
      // DFF setting
      //
      ys_dff_techmap = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/techlib/tech_flops.v";
      ys_dff_features.insert("async_reset");
      ys_dff_features.insert("async_set");
      ys_dff_features.insert("flop_enable");

      ys_dff_models["dffers"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffers.v";
      ys_dff_models["dffer"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffer.v";
      ys_dff_models["dffes"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffes.v";
      ys_dff_models["dffe"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffe.v";
      ys_dff_models["dffrs"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffrs.v";
      ys_dff_models["dffr"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffr.v";
      ys_dff_models["dffs"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dffs.v";
      ys_dff_models["dff"] = "+/plugins/yosys-syn/SRC/FF_MODELS/dff.v";


      // -------------------------
      // BRAM setting
      // 
      ys_brams_memory_libmap.clear();
      ys_brams_memory_libmap_parameters.clear();
      ys_brams_techmap.clear();

      if (bram_tech == "microchip") {

         // bram memory_libmap settings
	 //
         string brams_memory_libmap1 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/LSRAM.txt";
         string brams_memory_libmap2 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/uSRAM.txt";
	 ys_brams_memory_libmap.push_back(brams_memory_libmap1);
	 ys_brams_memory_libmap.push_back(brams_memory_libmap2);

	 string brams_memory_libmap_param1 = "-logic-cost-rom 0.5";
	 ys_brams_memory_libmap_parameters.push_back(brams_memory_libmap_param1);

	 string brams_memory_libmap_param2 = "-logic-cost-ram 1.0";
	 ys_brams_memory_libmap_parameters.push_back(brams_memory_libmap_param2);

         // bram techmap settings
	 //
         string brams_techmap1 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/LSRAM_map.v";
         string brams_techmap2 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/uSRAM_map.v";
	 ys_brams_techmap.push_back(brams_techmap1);
	 ys_brams_techmap.push_back(brams_techmap2);

      } else if (bram_tech == "zeroasic") {
  
         // bram memory_libmap settings
	 //
         string brams_memory_libmap1 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/memory_libmap.txt";
	 ys_brams_memory_libmap.push_back(brams_memory_libmap1);

         // bram techmap settings
	 //
         string brams_techmap1 = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/BRAM/techmap.v";
	 ys_brams_techmap.push_back(brams_techmap1);
      }
  
      // -------------------------
      // DSP setting
      //
      ys_dsps_techmap = "";
      ys_dsps_parameter_int.clear();
      ys_dsps_parameter_string.clear();
      ys_dsps_pack_command = "";

      if (dsp_tech == "xilinx") {

        ys_dsps_techmap = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/DSP/mult18x18_DSP48.v ";
        ys_dsps_parameter_int["DSP_A_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_B_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_A_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_B_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_Y_MINWIDTH"] = 8;
        ys_dsps_parameter_int["DSP_SIGNEDONLY"] = 1;
        ys_dsps_parameter_string["DSP_NAME"] = "$__MUL18X18";

        ys_dsps_pack_command = "dsp -family DSP48";

	return;

      } else if (dsp_tech == "microchip") {

        ys_dsps_techmap = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/DSP/polarfire_dsp_map.v ";
        ys_dsps_parameter_int["DSP_A_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_B_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_A_MAXWIDTH_PARTIAL"] = 18;  // Partial multipliers are intentionally
                                                               // limited to 18x18 in order to take
                                                               // advantage of the (PCOUT >> 17) -> PCIN
                                                               // dedicated cascade chain capability

        ys_dsps_parameter_int["DSP_A_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_B_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_Y_MINWIDTH"] = 8;
        ys_dsps_parameter_int["DSP_SIGNEDONLY"] = 1;
        ys_dsps_parameter_string["DSP_NAME"] = "$__MUL18X18";

        ys_dsps_pack_command = "microchip_dsp -family polarfire";

	return;

      } else if (dsp_tech == "bare_mult") {

        ys_dsps_techmap = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/DSP/bare_mult_tech_dsp.v ";
        ys_dsps_parameter_int["DSP_A_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_B_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_A_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_B_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_Y_MINWIDTH"] = 9;
        ys_dsps_parameter_int["DSP_SIGNEDONLY"] = 1;
        ys_dsps_parameter_string["DSP_NAME"] = "$__dsp_block";

	return;

      } else if (dsp_tech == "mae") {

        ys_dsps_techmap = "+/plugins/yosys-syn/ARCHITECTURE/" + part_name + "/DSP/mae_tech_dsp.v ";
        ys_dsps_parameter_int["DSP_A_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_B_MAXWIDTH"] = 18;
        ys_dsps_parameter_int["DSP_A_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_B_MINWIDTH"] = 2;
        ys_dsps_parameter_int["DSP_Y_MINWIDTH"] = 9;
        ys_dsps_parameter_int["DSP_SIGNEDONLY"] = 1;
        ys_dsps_parameter_string["DSP_NAME"] = "$__dsp_block";

	return;
      }

      log_warning("Could not find any specific DSP tech settings with 'dsp_tech' = '%s'\n", 
                  dsp_tech.c_str());

    }

  }

  // -------------------------
  // check_options
  // -------------------------
  void check_options()
  {
     if (!config_file_success) {

       // Converting 'partname' to upper case only if part name is 
       // used as 'synth_fpga' option and not through config file. 
       // If we do that also for partname set from config file, we may 
       // have trouble since we may upper case 3rd party partnames and
       // we would have no way to refer to the exact partname name,
       // set from config file, therefore the check on 'config_file_success'.
       //
       std::transform (part_name.begin(), part_name.end(), 
		       part_name.begin(), ::toupper);
     }

     if (partnames.count(part_name) == 0) {
        log("ERROR: -partname '%s' is unknown.\n", part_name.c_str());
        log("       Available partnames are :\n");
        for (auto part_name : partnames) {
           log ("               - %s\n", part_name.c_str());
        }
        log_error("Please select a correct partname.\n");
    }

    if (dsp_arch.count(dsp_tech) == 0) {
        log("ERROR: -use_dsp_tech '%s' is unknown.\n", dsp_tech.c_str());
        log("       Available DSP architectures are :\n");
        for (auto dsp : dsp_arch) {
           log ("               - %s\n", dsp.c_str());
        }
        log_error("Please select a correct DSP architecture.\n");
    }

    if (bram_arch.count(bram_tech) == 0) {
        log("ERROR: -use_bram_tech '%s' is unknown.\n", bram_tech.c_str());
        log("       Available BRAM architectures are :\n");
        for (auto bram : bram_arch) {
           log ("               - %s\n", bram.c_str());
        }
        log_error("Please select a correct BRAM architecture.\n");
    }

    if ((sc_syn_lut_size != "4") && (sc_syn_lut_size != "6")) {
        log_error("Lut sizes can be only 4 or 6.\n");
    }

    if ((sc_syn_fsm_encoding != "one-hot") && (sc_syn_fsm_encoding != "binary")) {
        log_error("-fsm_encoding can be only 'one-hot' or 'binary'.\n");
    }
  }

  // -------------------------
  // read_config
  // -------------------------
  // Read eventually config file that will setup main synthesis parameters like
  // partname, lut size, DFF models, DSP and BRAM techmap files, ...
  //
  // This should be in sync with the 'config_type' type since we will fill up
  // this data structure in this 'read_config' function.
  //
  void read_config() 
  {

    // if no 'config_file' specified return right away
    //
    if (config_file == "") {
      return;
    }

    log_header(G_design, "Reading config file '%s'\n", config_file.c_str());

    if (!std::filesystem::exists(config_file.c_str())) {
      log_error("Cannot find file '%s'.\n", config_file.c_str());
    }

    // Read the config file
    // 
    std::ifstream f;

    f.open(config_file);

    int line = 1;
    JsonNode root(f, config_file, line);

    // Analyze the 'root' config data structre and fill up 'G_config' with it
    //
    if (root.type != 'D') {
      log_error("'%s' file is not a dictionary.\n", config_file.c_str());
    }

    // Check that all sections are there and performs type verification.
    //

    // version
    //
    if (root.data_dict.count("version") == 0) {
        log_error("'version' number is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *version = root.data_dict.at("version");
    if (version->type != 'N') {
        log_error("'version' must be an integer.\n");
    }
    
    // root_path
    //
    JsonNode *root_path = NULL;
    if (root.data_dict.count("root_path") == 0) {
      log("NOTE: no 'root_path' section found in the config file : %s.\n",
          config_file.c_str());
    } else {
        root_path = root.data_dict.at("root_path");
        if (root_path->type != 'S') {
            log_error("'root_path' must be a string.\n");
        }
    }

    // partname
    //
    if (root.data_dict.count("partname") == 0) {
        log_error("'partname' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *partname = root.data_dict.at("partname");
    if (partname->type != 'S') {
        log_error("'partname' must be a string.\n");
    }

    // lut_size
    //
    if (root.data_dict.count("lut_size") == 0) {
        log_error("'lut_size' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *lut_size = root.data_dict.at("lut_size");
    if (lut_size->type != 'N') {
        log_error("'lut_size' must be an integer.\n");
    }

    // flipflops
    if (root.data_dict.count("flipflops") == 0) {
        log_error("'flipflops' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *flipflops = root.data_dict.at("flipflops");
    if (flipflops->type != 'D') {
        log_error("'flipflops' must be a dictionnary.\n");
    }

    // brams
    //
    JsonNode *brams = NULL;
    if (root.data_dict.count("brams") == 0) {
        log_warning("'brams' is missing in config file '%s'.\n", config_file.c_str());
    } else {
        brams = root.data_dict.at("brams");
        if (brams->type != 'D') {
            log_error("'brams' must be a dictionnary.\n");
        }
    }

    // dsps
    //
    JsonNode *dsps = NULL;
    if (root.data_dict.count("dsps") == 0) {
        log_warning("'dsps' is missing in config file '%s'.\n", config_file.c_str());
    } else {
       dsps = root.data_dict.at("dsps");
       if (dsps->type != 'D') {
           log_error("'dsps' must be a dictionnary.\n");
       }
    }

    // Extract data and fill up 'G_config'
    //
    G_config.config_file = config_file;

    G_config.version = version->data_number;

    if (!root_path) {

       G_config.root_path = "";

    } else {

       G_config.root_path = root_path->data_string;
    }

    G_config.partname = partname->data_string;

    G_config.lut_size = lut_size->data_number;

    // In case there was no explicit setting on "root_path" section
    // in the config file then pick up the config file location.
    //
    if (G_config.root_path == "") {

      const std::filesystem::path config_path(std::filesystem::absolute(config_file));

      G_config.root_path = config_path.parent_path();

      log("NOTE: Pick up Config file location as root path : %s\n", (G_config.root_path).c_str());
    }

    // Extract DFF associated parameters
    //
    if (flipflops->data_dict.count("features") == 0) {
        log_error("'features' from 'flipflops' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *dff_features = flipflops->data_dict.at("features");
    if (dff_features->type != 'A') {
        log_error("'features' associated to 'flipflops' must be an array.\n");
    }

    for (auto it : dff_features->data_array) {
          JsonNode *dff_mode = it;
          if (dff_mode->type != 'S') {
              log_error("Array associated to DFF 'features' must be contain only strings.\n");
          }
	  string dff_mode_str = dff_mode->data_string;

	  (G_config.dff_features).insert(dff_mode_str);
    }

    if (flipflops->data_dict.count("models") == 0) {
        log_error("'models' from 'flipflops' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *dff_models = flipflops->data_dict.at("models");
    if (dff_models->type != 'D') {
        log_error("'models' associated to 'flipflops' must be a dictionnary.\n");
    }

    for (auto it : dff_models->data_dict) {
	  string dff_model_str = it.first;
	  JsonNode* dff_model_path = it.second;
          if (dff_model_path->type != 'S') {
              log_error("Second element associated to DFF models '%s' must be a string.\n",
                        dff_model_str.c_str());
          }
	  G_config.dff_models[dff_model_str] = dff_model_path->data_string;
    }


    if (flipflops->data_dict.count("techmap") == 0) {
        log_error("'techmap' from 'flipflops' is missing in config file '%s'.\n", config_file.c_str());
    }
    JsonNode *techmap_dff = flipflops->data_dict.at("techmap");
    if (techmap_dff->type != 'S') {
        log_error("'techmap' associated to 'flipflops' must be a string.\n");
    }
    G_config.dff_techmap = techmap_dff->data_string;

    
    // ------------------------------------------
    // Extract 'brams' associated parameters
    // 
    if (!brams || (brams->data_dict.count("memory_libmap") == 0)) {
        log_warning("'memory_libmap' from 'brams' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no BRAM support.\n");
        G_config.brams_memory_libmap.clear();

    } else {

       JsonNode *memory_libmap = brams->data_dict.at("memory_libmap");
       if (memory_libmap->type != 'A') {
           log_error("'memory_libmap' associated to 'brams' must be an array.\n");
       }

       for (auto it : memory_libmap->data_array) {
          JsonNode *memory_libmap_path = it;
          if (memory_libmap_path->type != 'S') {
              log_error("Array associated to 'memory_libmap' must be contain only strings.\n");
          }
	  string memory_libmap_path_str = memory_libmap_path->data_string;

	  (G_config.brams_memory_libmap).push_back(memory_libmap_path_str);
       }
    }

    if (!brams || (brams->data_dict.count("memory_libmap_parameters") == 0)) {
        G_config.brams_memory_libmap_parameters.clear();

    } else {

       JsonNode *memory_libmap_parameters = brams->data_dict.at("memory_libmap_parameters");
       if (memory_libmap_parameters->type != 'A') {
           log_error("'memory_libmap_parameters' associated to 'brams' must be an array.\n");
       }

       for (auto it : memory_libmap_parameters->data_array) {
          JsonNode *memory_libmap_parameters_path = it;
          if (memory_libmap_parameters_path->type != 'S') {
              log_error("Array associated to 'memory_libmap_parameters' must be contain only strings.\n");
          }
          string memory_libmap_parameters_path_str = memory_libmap_parameters_path->data_string;

          (G_config.brams_memory_libmap_parameters).push_back(memory_libmap_parameters_path_str);
       }
    }


    if (!brams || (brams->data_dict.count("techmap") == 0)) {
        log_warning("'techmap' from 'brams' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no BRAM support.\n");
        G_config.brams_techmap.clear();

    } else {

       JsonNode *brams_techmap = brams->data_dict.at("techmap");
       if (brams_techmap->type != 'A') {
           log_error("'techmap' associated to 'brams' must be an array.\n");
       }

       for (auto it : brams_techmap->data_array) {
          JsonNode *bram_techmap_path = it;
          if (bram_techmap_path->type != 'S') {
              log_error("Array associated to 'techmap' must be contain only strings.\n");
          }
          string bram_techmap_path_str = bram_techmap_path->data_string;

          (G_config.brams_techmap).push_back(bram_techmap_path_str);
       }
    }


    // ------------------------------------------
    // Extract 'dsps' associated information
    // 
    
    // DSP Family
    //
    if (!dsps || (dsps->data_dict.count("family") == 0)) {
        log_warning("'family' from 'dsps' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no DSP support.\n");
        G_config.dsps_family = "";

    } else {

       JsonNode *family = dsps->data_dict.at("family");
       if (family->type != 'S') {
           log_error("'family' associated to 'dsps' must be a string.\n");
       }
       G_config.dsps_family = family->data_string;
    }


    // DSP techmap file
    //
    if (!dsps || (dsps->data_dict.count("techmap") == 0)) {
        log_warning("'techmap' from 'dsps' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no DSP support.\n");
        G_config.dsps_techmap = "";

    } else {

       JsonNode *dsps_techmap = dsps->data_dict.at("techmap");
       if (dsps_techmap->type != 'S') {
           log_error("'techmap' associated to 'dsps' must be a string.\n");
       }
       G_config.dsps_techmap = dsps_techmap->data_string;
    }


    // DSP techmap parameters : parameters can be string or int.
    //
    if (!dsps || (dsps->data_dict.count("techmap_parameters")) == 0) {
        log_warning("'techmap_parameters' from 'dsps' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no DSP support.\n");

    } else {

       JsonNode *dsps_param = dsps->data_dict.at("techmap_parameters");
       if (dsps_param->type != 'D') {
           log_error("'techmap_parameters' associated to 'dsps' must be a dictionnary.\n");
       }

       for (auto it : dsps_param->data_dict) {
          string param_str = it.first;
          JsonNode* param_value = it.second;

          if ((param_value->type != 'S') && (param_value->type != 'N')) {
              log_error("Second element associated to dsps 'techmap_parameters' '%s' must be either a string or an integer.\n",
                        param_str.c_str());
          }
	  if (param_value->type == 'S') {
            G_config.dsps_parameter_string[param_str] = param_value->data_string;
	    continue;
	  }
	  if (param_value->type == 'N') {
            G_config.dsps_parameter_int[param_str] = param_value->data_number;
	    continue;
	  }
	  log_warning("Ignoring 'dsps' parameter '%s'\n", param_str.c_str());
       }
    }
    
    // DSP pack command.
    //
    if (!dsps || (dsps->data_dict.count("pack_command")) == 0) {
        log_warning("'pack_command' from 'dsps' is missing in config file '%s'.\n", config_file.c_str());
        log_warning("Assuming that this technology has no DSP packing support.\n");

    } else {
       JsonNode *dsps_pack_command = dsps->data_dict.at("pack_command");
       if (dsps_pack_command->type != 'S') {
           log_error("'pack_command' associated to 'dsps' must be a string.\n");
       }
       G_config.dsps_pack_command = dsps_pack_command->data_string;
    }

    log_header(G_design, "Reading config file '%s' done with success !\n", config_file.c_str());

    config_file_success = true;

    show_config_file();
  }

  // ---------------------------------------
  // General object for XOR trees analysis
  // ---------------------------------------
  int sigspec_id = 0;
  dict<RTLIL::SigSpec, int> sigspec_ids;
  dict<RTLIL::SigSpec, int> y2height;
  dict<RTLIL::SigSpec, dict<RTLIL::SigSpec, Cell*>*> A2xors;

  class leaf_info {

    public : 
        RTLIL::SigSpec leaf;
        int Id;
  };
 
  class xor_head {

    public :
        Cell* xor_cell;
        RTLIL::SigSpec head;
        int height;
        pool<RTLIL::SigSpec> leaves;
        vector<leaf_info*> sorted_leaves;
        vector<RTLIL::SigSpec> sleaves;
  };

  // -------------------------
  // getHeight
  // -------------------------
  // get the height from the XOR output 'y', e.g number of XORs traversed till 
  // reaching a non XOR cell.
  //
  int getHeight(RTLIL::SigSpec& y, dict<RTLIL::SigSpec, Cell*>& y2xor)
  {
    // If 'y' is not a XOR Y output it is a XOR tree leaf
    //
    if (y2xor.count(y) == 0) {
       return 0;
    }

    Cell* cell = y2xor[y];

    if (cell->type != RTLIL::escape_id("$_XOR_")) {
       log_error("Expected to access a XOR cell !\n");
       return 0;
    }

    RTLIL::SigSpec A = cell->getPort(ID::A);
    RTLIL::SigSpec B = cell->getPort(ID::B);

    int height = 0;
    int heightA = 0;
    int heightB = 0;

    heightA = getHeight(A, y2xor); 
    heightB = getHeight(B, y2xor); 

    height = heightA;

    if (heightB > heightA) {
       height = heightB;
    }

    return height+1;
  }

  // -------------------------
  // getLeavesIds
  // -------------------------
  void getLeavesIds(RTLIL::SigSpec y, dict<RTLIL::SigSpec, Cell*>& y2xor, xor_head* xh)
  {
    // If 'y' is not a XOR Y output it is a XOR tree leaf
    //
    if (y2xor.count(y) == 0) {

       // If this 'y' leaf has been already visited then set this info.
       //
       if ((xh->leaves).find(y) != (xh->leaves).end()) {
         //log_warning("Found duplicated leaf during XOR tree analysis\n");
       }

       // Insert 'y' in the leaves of 'xh' (the XOR head)
       //
       xh->leaves.insert(y);

       // Associate an Id to this leaf if not visited yet.
       //
       if (sigspec_ids.find(y) == sigspec_ids.end()) {
         sigspec_ids[y] = sigspec_id++; 
       }

       return;
    }

    Cell* cell = y2xor[y];

    if (cell->type != RTLIL::escape_id("$_XOR_")) {
       log_error("Expected to access a XOR cell !\n");
       return;
    }

    RTLIL::SigSpec A = cell->getPort(ID::A);
    RTLIL::SigSpec B = cell->getPort(ID::B);

    // Perform recursive exploration with highest child first.
    // If same heights, visit A before B.
    //
    
    // If both A and B are terminal leaves
    //
    if ((y2height.find(A) == y2height.end()) &&
        (y2height.find(B) == y2height.end())) {

       getLeavesIds(A, y2xor, xh);
       getLeavesIds(B, y2xor, xh);

       return;
    }

    // If A is terminal leaf
    //
    if (y2height.find(A) == y2height.end()) {

       getLeavesIds(B, y2xor, xh);
       getLeavesIds(A, y2xor, xh);

       return;
    }

    // If B is terminal leaf
    //
    if (y2height.find(B) == y2height.end()) {

       getLeavesIds(A, y2xor, xh);
       getLeavesIds(B, y2xor, xh);

       return;
    }

    // If both A and B are not terminal leaves
    //
    int heightA = y2height[A];
    int heightB = y2height[B];

    // Visit highest child first
    //
    if (heightA >= heightB) {

       getLeavesIds(A, y2xor, xh);
       getLeavesIds(B, y2xor, xh);

       return;
    }

    getLeavesIds(B, y2xor, xh);
    getLeavesIds(A, y2xor, xh);
  }


  // -------------------------
  // build_binary_xor_tree_rec
  // -------------------------
  void build_binary_xor_tree_rec (Module* top_mod, RTLIL::SigSpec& y, 
		             dict<RTLIL::SigSpec, Cell*>& y2xor, 
			     RTLIL::SigSpec& new_y, 
			     vector<RTLIL::SigSpec>& leaves)
  {

    if (leaves.size() < 2) {
      log_error("Xor tree binary build requires at leat 2 leaves : %ld\n",
	        leaves.size());
    }

    vector<RTLIL::SigSpec> current_leaves(leaves);

    // Build a binary XOR tree and proceed level per level (breadth first approach)
    // Cut 'current_leaves' into sets of size 2 to build a XOR2.
    // Add the output of this new XOR2 into next 'next_leaves', list that will 
    // correspond to next stage of leaves to cut into sets of 2.
    //
    while (1) {

       vector<RTLIL::SigSpec> next_leaves;

       int i = 0; // 'i' will be either 0 or 1

       RTLIL::SigSpec A;
       RTLIL::SigSpec B;

       // Visit all the leaves and pack them by 2 to create a XOR2.
       //
       for (auto it : current_leaves) {
         
          if (i == 0) {

              A = it;
	      i++;
	      continue;
	  }

          B = it;

	  // Ok, we got the two next leaves A and B so now try to create the new 
	  // cell XOR(A,B)
	  //
	  RTLIL::Cell *new_cell;

	  // check for already existing XOR(A,B) 
	  //
	  if ((A2xors.find(A) != A2xors.end()) && 
              ((A2xors[A])->find(B) != (A2xors[A])->end())) {

            //log("Found already built XOR !\n");

            dict<RTLIL::SigSpec, Cell*>* B2xors = A2xors[A]; 

	    new_cell = (*B2xors)[B];

            new_y = new_cell->getPort(ID::Y);

	    next_leaves.push_back(RTLIL::SigSpec(new_y));

	    i = 0;

	    continue;
	  } 
	  
	  // Check for already existing XOR(B,A)
	  //
	  if ((A2xors.find(B) != A2xors.end()) && 
              ((A2xors[B])->find(A) != (A2xors[B])->end())) {

            //log("Found already built XOR case 2 !\n");

            dict<RTLIL::SigSpec, Cell*>* B2xors = A2xors[B]; 

	    new_cell = (*B2xors)[A];

            new_y = new_cell->getPort(ID::Y);

	    next_leaves.push_back(RTLIL::SigSpec(new_y));

	    i = 0;

	    continue;

	  } 
	  
	  // We need to create a new XOR
	  //
          Wire* new_wire = top_mod->addWire(NEW_ID);

          new_y = new_wire;

          new_cell = top_mod->addCell(NEW_ID, RTLIL::escape_id("$_XOR_"));

          new_cell->setPort(ID::A, A);
          new_cell->setPort(ID::B, B);

          new_cell->setPort(ID::Y, RTLIL::SigSpec(new_y));

	  next_leaves.push_back(RTLIL::SigSpec(new_y));

	  // Add the new XOR in the cache to eventually reuse it.
	  //
	  if (A2xors.find(A) != A2xors.end()) {

              dict<RTLIL::SigSpec, Cell*>* B2xors = A2xors[A]; 
	      (*B2xors)[B] = new_cell;

	  } else {

              dict<RTLIL::SigSpec, Cell*>* B2xors = new dict<RTLIL::SigSpec, Cell*>; 
	      (*B2xors)[B] = new_cell;
	      A2xors[A] = B2xors;
	  }

	  i = 0;

       } // end of 'for current_leaves' 

       // All the leaves have been visited.
       // If i == 1 this is the case of odd numbers in the 'current_leaves'.
       // Add it to 'next_leaves' to reconsider it in next iteration.
       //
       if (i == 1) {
	  next_leaves.push_back(RTLIL::SigSpec(A));
       }

       // Move the 'next_leaves' objects to 'current_leaves' to restart a new iteration.
       //
       current_leaves.clear();
       for (auto l : next_leaves) {
          current_leaves.push_back(l);
       }
       next_leaves.clear();

       // Terminal case in the while loop : there is only 1 leaf, e.g the Y ouput of the
       // root XOR : we are done.
       //
       if (current_leaves.size() == 1) {
         return;
       }
    }
  }

  // -------------------------
  // build_binary_xor_tree
  // -------------------------
  void build_binary_xor_tree (Module* top_mod, RTLIL::SigSpec& y, 
		             dict<RTLIL::SigSpec, Cell*>& y2xor, 
			     vector<RTLIL::SigSpec>& leaves)
  {
    if (leaves.size() < 4) {
      return;
    }

    Cell* cell = y2xor[y];

    vector<RTLIL::SigSpec> first_half;
    vector<RTLIL::SigSpec> second_half;

    int i = 0;

    for (auto it : leaves) {

       if (i < leaves.size()/2) {

         first_half.push_back(it);

       } else {

         second_half.push_back(it);
       }
       i++;
    }

    RTLIL::SigSpec A = cell->getPort(ID::A);

    RTLIL::SigSpec new_A;

    build_binary_xor_tree_rec(top_mod, A, y2xor, new_A, first_half);

    RTLIL::SigSpec B = cell->getPort(ID::B);

    RTLIL::SigSpec new_B;

    build_binary_xor_tree_rec(top_mod, B, y2xor, new_B, second_half);

    cell->setPort(ID::A, new_A);

    cell->setPort(ID::B, new_B);
  }

  // -------------------------
  // cmpHeight
  // -------------------------
  static bool cmpHeight (xor_head* a, xor_head* b)
  {
    if (a->height > b->height) {
      return true;
    }

    return false;
  }

  // -------------------------
  // cmpInvHeight
  // -------------------------
  static bool cmpInvHeight (xor_head* a, xor_head* b)
  {
    if (a->height < b->height) {
      return true;
    }

    return false;
  }

  // -------------------------
  // cmpId
  // -------------------------
  static bool cmpId (leaf_info* a, leaf_info* b)
  {
    if (a->Id < b->Id) {
      return true;
    }

    return false;
  }

static std::string id(RTLIL::IdString internal_id)
{
        const char *str = internal_id.c_str();
        return std::string(str);
}

static void dump_const(const RTLIL::Const &data, int width = -1, int offset = 0, bool no_decimal = false, bool escape_comment = false)
{
        log("CONSTANT");
}

static void dump_sigchunk(const RTLIL::SigChunk &chunk, bool no_decimal = false)
{
    if (chunk.wire == NULL) {
        dump_const(chunk.data, chunk.width, chunk.offset, no_decimal);
        return;
    }

    if (chunk.width == chunk.wire->width && chunk.offset == 0) {

        log("%s", id(chunk.wire->name).c_str());

    } else if (chunk.width == 1) {

        if (chunk.wire->upto)
            log("%s[%d]", id(chunk.wire->name).c_str(),
                (chunk.wire->width - chunk.offset - 1) + chunk.wire->start_offset);
        else
            log("%s[%d]", id(chunk.wire->name).c_str(), chunk.offset + chunk.wire->start_offset);

    } else {

        if (chunk.wire->upto)
             log("%s[%d:%d]", id(chunk.wire->name).c_str(),
                 (chunk.wire->width - (chunk.offset + chunk.width - 1) - 1) + chunk.wire->start_offset,
                 (chunk.wire->width - chunk.offset - 1) + chunk.wire->start_offset);
        else
             log("%s[%d:%d]", id(chunk.wire->name).c_str(),
                 (chunk.offset + chunk.width - 1) + chunk.wire->start_offset,
                 chunk.offset + chunk.wire->start_offset);
    }
}

static void show_sig(const RTLIL::SigSpec &sig)
{
        if (GetSize(sig) == 0) {
           log("{0{1'b0}}");
           return;
        }

        if (sig.is_chunk()) {

            dump_sigchunk(sig.as_chunk());

        } else {

            log("{ ");

            for (auto it = sig.chunks().rbegin(); it != sig.chunks().rend(); ++it) {

                 if (it != sig.chunks().rbegin())
                    log(", ");

                 dump_sigchunk(*it, true);
            }
            log(" }");
        }
}


  // -------------------------
  // binary_decomp_xor_trees
  // -------------------------
  // Tries to reduce XOR trees depth by binarizing the XOR trees.
  // Binarize long XOR tree chains in delay mode. Typical example is 'gray2bin'
  // in 'logikbench/basic' suite having 64 levels that can be reduced to 6 levels.
  //
  void binary_decomp_xor_trees(Module* top_mod)
  {
    dict<RTLIL::SigSpec, Cell*> y2xor;

    log_header(G_design, "Analyze XOR trees\n");

    run("stat");

    run(stringf("write_verilog -norename -noexpr -nohex -nodec before_binary_decomp_xor_trees.verilog"));

#if 0
    run(stringf("write_blif before_binary_decomp_xor_trees.blif"));
#endif

    // Reset global objects
    //
    sigspec_id = 0;
    sigspec_ids.clear();
    y2xor.clear();
    y2height.clear();
    A2xors.clear();

    // Build "y2xor" to get direct XOR cell access from any  
    // XOR cell 'y' output signal.
    //
    for (auto cell : top_mod->cells()) {

       if (cell->type != RTLIL::escape_id("$_XOR_")) {
         continue;
       }

       RTLIL::SigSpec Y = cell->getPort(ID::Y);

       y2xor[Y] = cell;
    }

    log("Found %ld xors\n", y2xor.size());


    // -----------------------------------
    // Compute the heights for each XORs
    // and get the 'heads" : a head corresponds to a XOR
    // from where we start the search in its Transitive Fanin.
    // So we have as many 'heads' as XORs.
    // In a 'head' we store the 'y' XOR output, the max height till
    // a leaf (a non XOR), the vector of its leaves (all the non XOR
    // terminals).
    //
    int maxHeight = 0;
    vector<xor_head*> heads;

    for (auto it : y2xor) {

       RTLIL::SigSpec y = it.first;

       int height = getHeight(y, y2xor);

       xor_head* xh = new xor_head;

       Cell* xor_cell = y2xor[y]; // must exist based on previous loop

       xh->xor_cell = xor_cell;

       xh->head = y;

       xh->height = height;

       heads.push_back(xh);

       y2height[y] = height;

       if (height > maxHeight) {
          maxHeight = height;
       }
    }

    log("Max Xor tree height = %d\n", maxHeight);

    // Sort XOR heads from highest to lowest
    //
    std::sort(heads.begin(), heads.end(), cmpHeight);

    // ------------------------------------------------------------
    // From Highest head to lowest : get leaves and set leaves Ids
    // for each 'head'.
    // Idea: gives first IDs to deepest leaves.
    //
    for (auto xh : heads) {

       RTLIL::SigSpec y = xh->head;
       int height = xh->height;

       getLeavesIds(y, y2xor, xh);
    }

    // When binarizing the highest XOR tree, its depth after binarizing
    // will be log2(max #leaves). We cannot do better. So it is not 
    // necessary to try to reduce/binarize original XOR trees with 
    // height <= log2(max #leaves).
    //
    double stuck_max_height = 0; // the height we cannot go below even by
                                 // reducing height with binary tree.

    //std::sort(heads.begin(), heads.end(), cmpInvHeight);

    // Binarize the xor logic underneath 'y'
    //
    for (auto it : heads) {

       RTLIL::SigSpec y = it->head;
       int height = it->height;
       pool<RTLIL::SigSpec> leaves = it->leaves;

#if 0
       if (height <= stuck_max_height) {
          log("Stop binarize XORs because stuck height is '%f' '%d'\n", 
              stuck_max_height, height);

	  continue;
       }
#endif

       // Create the "sorted_leaves" for each XOR header.
       // in order to sort them.
       //
       for (auto lv : leaves) {

	 leaf_info* lf = new leaf_info;

	 lf->leaf = lv;

	 lf->Id = sigspec_ids[lv];

         it->sorted_leaves.push_back(lf);
       }

       vector<leaf_info*> sorted_leaves = it->sorted_leaves;

       // Sort the leaves according to their IDs
       //
       std::sort(sorted_leaves.begin(), sorted_leaves.end(), cmpId);

       // Construct the 'sleaves' to use for building the XOR bin tree.
       // "sleaves' is simply the image of "sorted_leaves" w/o the Ids.
       //
       for (auto lv : sorted_leaves) {
          it->sleaves.push_back(lv->leaf);
       }

       // Build the binary tree under XOR with output 'y'
       //
       build_binary_xor_tree(top_mod, y, y2xor, it->sleaves);

#if 0
       if (stuck_max_height <= log2(it->leaves.size())) {

         stuck_max_height = log2(it->leaves.size());

         log("New Stuck max height = %f\n", stuck_max_height);
       } 
#endif
    }

    // Dispose objects created by 'new'.
    //
    for (auto xh : heads) {
        delete xh;
    }
    heads.clear();

    for (auto ax : A2xors) {
        delete ax.second;
    }
    A2xors.clear();

    run("opt_merge"); // should merge the remaining duplicated XORs

    run("opt_clean");

    // Look for duplicate XORs
    //

    int nb_duplicate = 0;

    for (auto cell : top_mod->cells()) {

       if (cell->type != RTLIL::escape_id("$_XOR_")) {
         continue;
       }

       RTLIL::SigSpec A = cell->getPort(ID::A);
       RTLIL::SigSpec B = cell->getPort(ID::B);
       RTLIL::SigSpec Y = cell->getPort(ID::Y);

       if (A2xors.find(A) != A2xors.end()) { // look for XOR A B

           dict<RTLIL::SigSpec, Cell*>* B2xors = A2xors[A];

           if (B2xors->find(B) != B2xors->end()) {
             log("Found duplicated XOR with same A and B\n");
	     nb_duplicate++;
	     continue;
	   }
           (*B2xors)[B] = cell;

       } else if (A2xors.find(B) != A2xors.end()) { // look for XOR B A

           dict<RTLIL::SigSpec, Cell*>* a2xors = A2xors[B];

           if (a2xors->find(A) != a2xors->end()) {
             log("Found duplicated XOR with same B and A\n");
	     nb_duplicate++;
	     continue;
	   }
           (*a2xors)[A] = cell;

       } else {

           dict<RTLIL::SigSpec, Cell*>* B2xors = new dict<RTLIL::SigSpec, Cell*>;
           (*B2xors)[B] = cell;
           A2xors[A] = B2xors;
       }
    }

    log("Found '%d' duplicated XORs after binarization.\n", nb_duplicate);

    run("stat");

#if 0
    run(stringf("write_blif after_binary_decomp_xor_trees.blif"));
#endif

  }

  // -------------------------
  // analyze_undriven_nets
  // -------------------------
  void analyze_undriven_nets(Module* top_mod, bool connect_to_undef) 
  {
    pool<SigBit> undriven_bits;
    SigMap assign_map;
    CellTypes ct;

    if (!G_design) {
      return;
    }

    log_header(G_design, "Analyze undriven nets\n");

#if 0
    run(stringf("write_verilog -norename -noexpr -nohex -nodec %s", "before_undriven_cleanup.verilog"));
#endif

    ct.setup(G_design);

    undriven_bits.clear();
    assign_map.set(top_mod);

    for (auto wire : top_mod->wires()) {
        for (auto bit : assign_map(wire)) {
             if (bit.wire) {
               undriven_bits.insert(bit);
	     }
	}
    }

    for (auto wire : top_mod->wires()) {
        if (wire->port_input) {
              for (auto bit : assign_map(wire)) {
                undriven_bits.erase(bit);
	      }
	}
    }

    for (auto cell : top_mod->cells()) {
        for (auto &conn : cell->connections()) {

           if (!ct.cell_known(cell->type) || ct.cell_output(cell->type, conn.first)) {
                 for (auto bit : assign_map(conn.second)) {
                    undriven_bits.erase(bit);
		 }
	   }
	}
    }


    SigSpec undriven_sig(undriven_bits);
    undriven_sig.sort_and_unify();

    for (auto chunk : undriven_sig.chunks()) {

       if (connect_to_undef) {

          log_warning("Setting to 'X' undriven net '%s'\n", log_signal(chunk));
          top_mod->connect(chunk, SigSpec(State::Sx, chunk.width));

       } else {
          log_warning("net %s is undriven\n", log_signal(chunk));
       }
    }

    if (stop_if_undriven_nets && undriven_sig.size()) {
      run(stringf("write_verilog -norename -noexpr -nohex -nodec %s", "netlist_synth_fpga.verilog"));
      log("\nDumping verilog file 'netlist_synth_fpga.verilog'\n");
      log("\n");
      log_error("Stop synthesis [-stop_if_undriven_nets is ON] : Final netlist has '%d' undriven nets !\n", 
                undriven_sig.size());
    }
  }

  // -------------------------
  // getNumberOfLuts
  // -------------------------
  int getNumberOfLuts() {

     int nb = 0;

     if (!G_design) {
       log_warning("Design seems empty !\n");
       return -1;
     }

     for (auto cell : G_design->top_module()->cells()) {
         if (cell->type.in(ID($lut))) {
           nb++;
         }
     }

     return nb;
  }

  // -------------------------
  // getNumberOfDffs
  // -------------------------
  int getNumberOfDffs() {

     int nb = 0;

     if (!G_design) {
       log_warning("Design seems empty !\n");
       return -1;
     }

     for (auto cell : G_design->top_module()->cells()) {
         if (cell->type.in(ID(dff), ID(dffe), ID(dffr), ID(dffer),
                           ID(dffs), ID(dffrs), ID(dffes), ID(dffers))) {
           nb++;
         }
     }

     return nb;
  }

  // -------------------------
  // dump_csv_file 
  // -------------------------
  void dump_csv_file(string fileName, int runTime)
  {
     if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
     }

     // -----
     // Get all the stats 
     //
     Module* topModule = G_design->top_module();

     if (!topModule) {
       log_warning("Design seems empty !\n");
       return;
     }

     string topName = log_id(topModule->name);

     int nbLuts = getNumberOfLuts();

     int nbDffs = getNumberOfDffs();

     int maxlvl = -1;

     // call 'max_level' command if not called yet
     //
     if (!show_max_level) {
         run("max_level -summary"); // -> store 'maxlvl' in scratchpad 
	                            // with 'max_level.max_levels'

	 maxlvl = G_design->scratchpad_get_int("max_level.max_levels", 0);
     }

     // -----
     // Open the csv file and dump the stats.
     //
     std::ofstream csv_file(fileName);

     csv_file << topName + ",";
     csv_file << std::to_string(nbLuts) + ",";
     csv_file << std::to_string(nbDffs) + ",";
     csv_file << std::to_string(maxlvl) + ",";
     csv_file << std::to_string(runTime);
     csv_file << std::endl;

     csv_file.close();

     log("\n   Dumped file %s\n", fileName.c_str());
  }

  // -------------------------
  // load_dff_bb_models 
  // -------------------------
  void load_dff_bb_models()
  {
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dff.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffe.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffr.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffs.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffrs.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffer.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffes.v");
     run("read_verilog +/plugins/yosys-syn/FF_MODELS/dffers.v");

     run("blackbox dff dffe dffr dffs dffrs dffer dffes dffers");
  }

  // -------------------------
  // dbg_wait 
  // -------------------------
  void dbg_wait ()
  {  
     if (wait) {
       getchar();
     }
  }

  // -------------------------
  // getNumberOfCells
  // -------------------------
  int getNumberOfCells() {
     return ((G_design->top_module()->cells()).size());
  }

  // -------------------------
  // clean_design 
  // -------------------------
  void clean_design(int use_dff_bb_models)
  {
     if (obs_clean) {

        run("splitcells");

        run("splitnets");

	if (use_dff_bb_models) {

	  // Load black box models to get IOs directions for 
	  // 'obs_clean'
	  //
          load_dff_bb_models();
	}

        //run("obs_clean");

        run("hierarchy");

        run("stat");

     } else {

        run("opt_clean");
     }
  }

  // -------------------------
  // abc_synthesize 
  // -------------------------
  void abc_synthesize()
  {
    if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
    }

    run("stat");

    if (opt == "") {
       log_header(G_design, "Performing OFFICIAL PLATYPUS optimization\n");
       run("abc -lut " + sc_syn_lut_size);
       return;
    }

#if 0
    run("show -prefix before_abc");
#endif

    string mode = opt;

    // Switch to FAST ABC synthesis script for huge designs in order to avoid
    // runtime blow up.
    //
    // ex: 'ifu', 'l2c', 'e203' designs will be impacted with sometime slight max
    // level degradation but with nice speed-up (ex: 'e203' from 5000 sec. downto
    // 1400 sec.)
    //
    int nb_cells = getNumberOfCells();

    if (nb_cells <= TINY_NB_CELLS) { 

      if (mode == "area") {

         mode  = "tiny_area";

      } else if (mode == "delay") {

         mode  = "tiny_delay";
      }

    } else if (nb_cells <= SMALL_NB_CELLS) { 

      if (mode == "area") {

         mode  = "small_area";

      } else if (mode == "delay") {

         mode  = "small_delay";
      }

    } else if (nb_cells >= HUGE_NB_CELLS) { // example : 'zmcml' from Golden suite

      mode  = "huge";

      log_warning("Optimization script changed from '%s' to '%s' due to design size (%d cells)\n",
                  opt.c_str(), mode.c_str(), nb_cells);

    } else if (nb_cells >= BIG_NB_CELLS) { // example : 'e203_soc_top' from Golden suite

      mode  = "fast";

      log_warning("Optimization script changed from '%s' to '%s' due to design size (%d cells)\n",
                  opt.c_str(), mode.c_str(), nb_cells);
    }

    // Otherwise specific ABC script based flow
    //
    string abc_script = "+/plugins/yosys-syn/ABC_SCRIPTS/LUT" + sc_syn_lut_size +
	                "/" + abc_script_version + "/" + mode + "_lut" + 
			sc_syn_lut_size + ".scr";

    log_header(G_design, "Calling ABC script in '%s' mode\n", mode.c_str());

    run("abc -script " + abc_script);
  }

  // -------------------------
  // legalize_flops
  // -------------------------
  // Code picked up from procedure 'legalize_flops' in TCL 
  // 'sc_synth_fpga.tcl' (Thierry)
  // DFF features considered so far : 
  //     - enable
  //     - async_set
  //     - async_reset
  //
  void legalize_flops ()
  {

    if (use_sdff) { // internal hidden mode to do QoR comparison with technology supporting
	            // DFF with synchronous set/reset.

      run("dfflegalize -cell $_DFFE_?P?P_ 01 -cell $_SDFFE_?P?P_ 01 -cell $_DLATCH_?P?_ 01", "(mimic xc7)");
      return;
    }

    // Consider all feature combinations 'enable" x "async_set' x 
    // 'async_reset' when features are supported or not : 2x2x2 = 8 
    // combinations to handle.
    //
    
    // 1.
    //
    if (dff_enable && dff_async_set && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN?_ $_DFFE_PP_ $_DFFE_PN?P_ $_DFFSR_PNN_ $_DFFSRE_PNNP_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN?_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN?P_ 01 -cell $_DFFSR_PNN_ 01 -cell $_DFFSRE_PNNP_ 01");
      return;
    }

    // 2.
    //
    if (dff_enable && dff_async_set) {
      log("Legalize list: $_DFF_P_ $_DFF_PN1_ $_DFFE_PP_ $_DFFE_PN1P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN1_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN1P_ 01");
      return;
    }

    // 3.
    //
    if (dff_enable && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN0_ $_DFFE_PP_ $_DFFE_PN0P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN0_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN0P_ 01");
      return;
    }

    // 4.
    //
    if (dff_enable) {
      log("Legalize list: $_DFF_P_ $_DFF_P??_ $_DFFE_PP_ $_DFFE_P??P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_P??_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_P??P_ 01");
      return;
    }

    // 5.
    //
    if (dff_async_set && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN?_ $_DFFSR_PNN_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN?_ 01 -cell $_DFFSR_PNN_ 01");
      return;
    }

    // 6.
    //
    if (dff_async_set) {
      log("Legalize list: $_DFF_P_ $_DFF_PN1_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN1_ 01");
      return;
    }

    // 7.
    //
    if (dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN0_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN0_ 01");
      return;
    }

    // 8.
    //
    // case of all features are not supported
    //

    log_warning("No DFF features are suported !\n");
    log_warning("Still Legalize list: $_DFF_P_\n");
    run("dfflegalize -cell $_DFF_P_ 01");

  }

  // -------------------------
  // infer_BRAMs
  // -------------------------
  // In order that BRAM inference kicks in, we need to have both "ys_brams_memory_libmap"
  // and "ys_brams_techmap" defined. It one of two is not defined then BRAM inference
  // is ignored.
  //
  void infer_BRAMs()
  {
     if (no_bram) {
       return;
     }

     if (ys_brams_memory_libmap.size() == 0) {
       if (ys_brams_techmap.size() != 0) {
          log_warning("BRAM inference ignored because no memory libmap file has been provided !\n");
       }
       return;
     }

     if (ys_brams_techmap.size() == 0) {
       if (ys_brams_memory_libmap.size() != 0) {
          log_warning("BRAM inference ignored because no memory techmap file has been provided !\n");
       }
       return;
     }

     // Build bram 'memory_map' command + arguments
     //
     string sc_syn_bram_memory_libmap = "memory_libmap";

     for (auto it : ys_brams_memory_libmap) {
	sc_syn_bram_memory_libmap += " -lib " + it;
     }
     for (auto it : ys_brams_memory_libmap_parameters) {
	sc_syn_bram_memory_libmap += " " + it;
     }

     // Build bram 'techmap' command + arguments
     //
     string sc_syn_bram_techmap = "techmap";
     for (auto it : ys_brams_techmap) {
	sc_syn_bram_techmap += " -map " + it;
     }

     run("stat");

#if 0
     log("Call %s\n", sc_syn_bram_memory_libmap.c_str());
     log("Call %s\n", sc_syn_bram_techmap.c_str());
     getchar();
#endif

     log("\nWARNING: Make sure you are using the right 'partname' for the BRAM inference in case of failure.\n");
     
     run(sc_syn_bram_memory_libmap);

     log("\nWARNING: Make sure you are using the right 'partname' for the BRAM inference in case of failure.\n");

     run(sc_syn_bram_techmap);

     run("stat");

  }

  // -------------------------
  // infer_DSPs
  // -------------------------
  void infer_DSPs()
  {
     if (no_dsp) {
       return;
     }

     if (ys_dsps_techmap == "") {
       return;
     }

     run("stat");

     run("memory_dff"); // 'dsp' will merge registers, reserve memory port registers first

     string sc_syn_dsps_techmap = "techmap -map +/mul2dsp.v -map " + ys_dsps_techmap + " ";

     for (auto it : ys_dsps_parameter_int) {
         sc_syn_dsps_techmap += "-D " + it.first + "=" + std::to_string(it.second) + " ";
     }
     for (auto it : ys_dsps_parameter_string) {
         sc_syn_dsps_techmap += "-D " + it.first + "=" + it.second + " ";
     }

     string sc_syn_dsps_pack_command = ys_dsps_pack_command; 

#if 0
     log("Call %s\n", sc_syn_dsps_techmap.c_str());
     getchar();
#endif

     log("\nWARNING: Make sure you are using the right 'partname' for the DSP inference in case of failure.\n");

     run(sc_syn_dsps_techmap);

     run("stat");

     run("select a:mul2dsp");
     run("setattr -unset mul2dsp");
     run("opt_expr -fine");
     run("wreduce");
     run("select -clear");

     // Call the DSP packer command
     //
     if (sc_syn_dsps_pack_command != "") {
        run(sc_syn_dsps_pack_command);
     }

     run("chtype -set $mul t:$__soft_mul");

     run("stat");
  }

  // -------------------------
  // resynthesize
  // -------------------------
  // Avoid the heavy synthesis flow and performs a light structural synthesis
  // because we are re-optimizing and re-mapping a netlist.
  //
  void resynthesize() 
  {
    run("stat");

    run("proc");

    run("flatten");

    run("techmap -map +/techmap.v");

    run("techmap");

    run("opt -fast");
    run("opt_clean");

    // Call light 'opt' if design is huge (ex: 'zmcml')
    //
    if (getNumberOfCells() <= HUGE_NB_CELLS) {
       run("opt -full");
    } else {
       run("opt_expr");
       run("opt_clean");
    }

    legalize_flops ();

    string sc_syn_flop_library = ys_dff_techmap;
    run("techmap -map " + sc_syn_flop_library);

    run("techmap");
    
    // Call light 'opt' if design is huge (ex: 'zmcml')
    //
    if (getNumberOfCells() <= HUGE_NB_CELLS) {
       run("opt -purge");
    } else {
       run("opt_expr");
       run("opt_clean");
    }

    if (insbuf) {
      run("insbuf");
    }

    abc_synthesize();

    run("setundef -zero");
    run("clean -purge");

    run("stat");
  }

  // -------------------------
  // coarse_synthesis
  // -------------------------
  void coarse_synthesis()
  {
    run("opt_expr");
    run("opt_clean");
    run("check");
    run("opt -nodffe -nosdff");
    run("fsm -encoding " + sc_syn_fsm_encoding);
    run("opt");
    run("wreduce");
    run("peepopt");
    run("opt_clean");
    run("share");
    run("techmap -map +/cmp2lut.v -D LUT_WIDTH=" + sc_syn_lut_size);
    run("opt_expr");
    run("opt_clean");

  }

  // -------------------------
  // help
  // -------------------------
  //
  void help() override
  {
	//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
	log("\n");
	log("    synth_fpga [options]\n");
	log("\n");
	log("This command runs Zero Asic FPGA synthesis flow.\n");
	log("\n");
	log("    -top <module>\n");
	log("        use the specified module as top module\n");
        log("\n");

	log("    -config <file name>\n");
	log("        Specifies the config file setting main 'synth_fpga' parameters.\n");
        log("\n");

	log("    -show_config\n");
	log("        Show the parameters set by the config file.\n");
        log("\n");

        log("    -no_flatten\n");
        log("        skip flatening. By default, design is flatened.\n");
        log("\n");

        log("    -opt\n");
        log("        specifies the optimization target : area, delay, default, fast.\n");
        log("\n");

        log("    -partname\n");
        log("        Specifies the Architecture partname used. By default it is Z1000.\n");
        log("\n");

        log("    -no_bram\n");
        log("        Bypass BRAM inference. It is off by default.\n");
        log("\n");

        log("    -use_bram_tech [zeroasic, microchip]\n");
        log("        Invoke architecture specific DSP inference. It is off by default. -no_BRAM \n");
        log("        overides -use_BRAM_TECH.\n");
        log("\n");

        log("    -no_dsp\n");
        log("        Bypass DSP inference. It is off by default.\n");
        log("\n");

        log("    -use_dsp_tech [xilinx, microchip, bare_mult, mae]\n");
        log("        Invoke architecture specific DSP inference. It is off by default. -no_DSP \n");
        log("        overides -use_dsp_tech.\n");
        log("\n");

        log("    -fsm_encoding [one-hot, binary]\n");
        log("        Specifies FSM encoding : by default a 'one-hot' encoding is performed.\n");
        log("\n");

        log("    -resynthesis\n");
        log("        switch synthesis flow to resynthesis mode which means a lighter flow.\n");
        log("        It can be used only after performing a first 'synth_fpga' synthesis pass \n");
        log("\n");

        log("    -insbuf\n");
        log("        performs buffers insertion (Off by default).\n");
        log("\n");

        log("    -no_xor_tree_process\n");
        log("        Disable xor trees depth reduction for DELAY (Off by default).\n");
        log("\n");

        log("    -autoname\n");
        log("        Generate, if possible, better wire and cells names close to RTL names rather than\n");
        log("        $abc generic names.\n");
        log("\n");

	// DFF related options
	//
        log("    -no_dff_enable\n");
        log("        specifies that DFF with enable feature is not supported. By default,\n");
        log("        DFF with enable is supported.\n");
        log("\n");
        log("    -no_dff_async_set\n");
        log("        specifies that DFF with asynchronous set feature is not supported. By default,\n");
        log("        DFF with asynchronous set is supported.\n");
        log("\n");
        log("    -no_dff_async_reset\n");
        log("        specifies that DFF with asynchronous reset feature is not supported. By default,\n");
        log("        DFF with asynchronous reset is supported.\n");
        log("\n");
        log("    -no_opt_sat_dff\n");
        log("        Disable SAT-based DFF optimizations. This is off by default.\n");
        log("\n");

        log("    -no_opt_const_dff\n");
        log("        Disable constant driven DFF optimization as it can create simulation differences (since it may ignore DFF init values in some cases). This is off by default.\n");
        log("\n");

        log("    -stop_if_undriven_nets\n");
        log("        Stop Synthesis if the final netlist has undriven nets.\n");

        log("\n");
        log("    -obs_clean\n");
        log("        specifies to use 'obs_clean' cleanup function instead of regular \n");
        log("        'opt_clean'.\n");
        log("\n");

        log("    -lut_size\n");
        log("        specifies lut size. By default lut size is 4.\n");
        log("\n");

        log("    -verilog <file>\n");
        log("        write the design to the specified Verilog netlist file. writing of an\n");
        log("        output file is omitted if this parameter is not specified.\n");
	log("\n");

        log("    -show_max_level\n");
        log("        Show longest paths.\n");
        log("\n");

        log("    -csv\n");
        log("        Dump a 'stat.csv' file.\n");
        log("\n");

        log("    -wait\n");
        log("        wait after each 'stat' report for user to touch <enter> key. Help for \n");
        log("        flow analysis/debug.\n");
        log("\n");
  }

  // -------------------------
  // clear_flags
  // -------------------------
  // set default values for global parameters
  //
  void clear_flags() override
  {
	top_opt = "-auto-top";
	opt = "";

	part_name = "Z1000";

	no_flatten = false;
	no_opt_sat_dff = false;
	autoname = false;

	dsp_tech = "microchip";
	no_dsp = false;

	bram_tech = "microchip";
	no_bram = false;
	use_sdff = false;

	resynthesis = false;
	show_config = false;
	show_max_level = false;
	csv = false;
	insbuf = false;

	no_xor_tree_process = false;
	no_opt_const_dff = false;

	wait = false;

	dff_enable = true;
	dff_async_set = true;
	dff_async_reset = true;

	obs_clean = false;

	stop_if_undriven_nets = false;

	verilog_file = "";

	abc_script_version = "BEST";

	sc_syn_lut_size = "4";
	sc_syn_fsm_encoding = "one-hot";
	config_file = "";
	config_file_success = false;
  }

  // -------------------------
  // execute
  // -------------------------
  //
  void execute(std::vector<std::string> args, RTLIL::Design *design) override
  {
	string run_from, run_to;
	clear_flags();

        log_header(design, "Executing 'synth_fpga'\n\n");

	G_design = design;

	size_t argidx;

	for (argidx = 1; argidx < args.size(); argidx++)
        {
          if (args[argidx] == "-top" && argidx+1 < args.size()) {
 	     top_opt = "-top " + args[++argidx];
	     continue;
	  }

          if (args[argidx] == "-config" && argidx+1 < args.size()) {
 	     config_file = args[++argidx];
	     continue;
	  }

          if (args[argidx] == "-show_config") {
             show_config = true;
             continue;
          }

	  if (args[argidx] == "-opt" && argidx+1 < args.size()) {
	     opt = args[++argidx];
	     if (opt_options.count(opt) == 0) {
                log_cmd_error("-opt option '%s' is unknown. Please see help.\n", 
                              (args[argidx]).c_str());
	     }
	     continue;
          }

          if (args[argidx] == "-resynthesis") {
             resynthesis = true;
             continue;
          }

          if (args[argidx] == "-no_flatten") {
             no_flatten = true;
             continue;
          }

          if (args[argidx] == "-no_opt_sat_dff") {
             no_opt_sat_dff = true;
             continue;
          }

          if (args[argidx] == "-no_bram") {
             no_bram = true;
             continue;
          }

          if (args[argidx] == "-use_sdff") { // hidden option
             use_sdff = true;
             continue;
          }

          if (args[argidx] == "-use_bram_tech" && argidx+1 < args.size()) {
             bram_tech = args[++argidx];
             continue;
          }

          if (args[argidx] == "-no_dsp") {
             no_dsp = true;
             continue;
          }

          if (args[argidx] == "-use_dsp_tech" && argidx+1 < args.size()) {
             dsp_tech = args[++argidx];
             continue;
          }

          if (args[argidx] == "-insbuf") {
             insbuf = true;
             continue;
          }

          if (args[argidx] == "-no_xor_tree_process") {
             no_xor_tree_process = true;
             continue;
          }

          if (args[argidx] == "-no_opt_const_dff") {
             no_opt_const_dff = true;
             continue;
          }

          if (args[argidx] == "-autoname") {
             autoname = true;
             continue;
          }

          if (args[argidx] == "-partname" && argidx+1 < args.size()) {
             part_name = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-lut_size" && argidx+1 < args.size()) {
             sc_syn_lut_size = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-fsm_encoding" && argidx+1 < args.size()) {
             sc_syn_fsm_encoding = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-abc_script_version" && argidx+1 < args.size()) {
             abc_script_version = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-stop_if_undriven_nets") {
             stop_if_undriven_nets = true;
             continue;
          }

	  if (args[argidx] == "-obs_clean") {
             obs_clean = true;
             continue;
          }

	  if (args[argidx] == "-verilog" && argidx+1 < args.size()) {
             verilog_file = args[++argidx];
             continue;
          }

          // Support of DFF features : with or without 
          //     - enable
          //     - async set
          //     - async reset
          //
          if (args[argidx] == "-no_dff_enable") {
             dff_enable = false;
             continue;
          }

          if (args[argidx] == "-no_dff_async_set") {
             dff_async_set = false;
             continue;
          }

          if (args[argidx] == "-no_dff_async_reset") {
             dff_async_reset = false;
             continue;
          }

          if (args[argidx] == "-show_max_level") {
             show_max_level = true;
             continue;
          }

          if (args[argidx] == "-csv") {
             csv = true;
             continue;
          }

          // for debug, flow analysis
          //
	  if (args[argidx] == "-wait") {
             wait = true;
             continue;
          }

          log_cmd_error("Unknown option : %s\n", (args[argidx]).c_str());
	}
        extra_args(args, argidx, design);

        if (!design->full_selection()) {
           log_cmd_error("This command only operates on fully selected designs!\n");
        }

        log_header(design, "Executing Zero Asic 'synth_fpga' flow.\n");
        log_push();

        run_script(design, run_from, run_to);

        log_pop();
  }

  // ---------------------------------------------------------------------------
  // script (synth_fpga flow) 
  // ---------------------------------------------------------------------------
  //
  // VERSION 1.0 (05/13/2025, Thierry): 
  //
  //        - as a starter, we mimic what is done in : 
  //          '.../siliconcompiler/tools/yosys/sc_synth_fpga.tcl' 
  //
  //        - we try to handle DFF legalization by taking care of DFF features
  //        support like handling DFF with 'enable', 'async_set', 'async_reset'.
  //
  // ---------------------------------------------------------------------------
  void script() override
  {

    if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    log("\nPLATYPUS flow using 'synth_fpga' Yosys plugin command\n");

    log("'Zero Asic' FPGA Synthesis Version : %s\n", SYNTH_FPGA_VERSION);

    // Read eventually config file that will setup main synthesis options like
    // partname, lut size, DFF models, DSP and BRAM techmap files, ...
    //
    read_config();

    // We setup the options according to 'config' file, if any, and command
    // line options.
    // If options conflict (ex: lut_size) between the two, 'config' file 
    // setting has the priority over 'synth_fpga' explicit option setting.
    //
    setup_options();

    // Check that all options are valid. 
    // Example : check that the 'partname' exists.
    //
    check_options();

    // Check hierarch and find the TOP
    //
    run(stringf("hierarchy -check %s", help_mode ? "-top <top>" : top_opt.c_str()));

    Module* topModule = G_design->top_module();

    if (!topModule) {
       log_warning("Design seems empty !\n");
       return;
    }

    // In case user invokes resynthsis at the command line level, 
    // performs a light weight synthesis for the second time.
    //
    if (resynthesis) {

       resynthesize();
       return;
    }

    run("proc");

    dbg_wait();

    if (!no_flatten) {
      run("flatten");
    }

    // Note there are two possibilities for how macro mapping might be done:
    // using the extract command (to pattern match user RTL against
    // the techmap) or using the techmap command.  The latter is better
    // for mapping simple multipliers; the former is better (for now)
    // for mapping more complex DSP blocks (MAC, pipelined blocks, etc).
    // and is also more easily extensible to arbitrary hard macros.
    // Run separate passes of both to get best of both worlds

    // An extract pass needs to happen prior to other optimizations,
    // otherwise yosys can transform its internal model into something
    // that doesn't match the patterns defined in the extract library

    // Other hard macro passes can happen after the generic optimization
    // passes take place.

    // Generic optimization passes; this is a fusion of the VTR reference
    // flow and the Yosys synth_ice40 flow
    //
    coarse_synthesis();

    // Extra line added versus 'sc_synth_fpga.tcl' tcl script version
    //
    run("stat");

    dbg_wait();

    // Here is a remaining customization pass for DSP tech mapping
    // Map DSP blocks before doing anything else,
    // so that we don't convert any math blocks
    // into other primitives
    //
    // Map DSP components
    //
    infer_DSPs();

    // Mimic ICE40 flow by running an alumacc and memory -nomap passes
    // after DSP mapping
    //  
    run("alumacc");
    run("opt");
    run("memory -nomap");

    run("design -save copy");

    // First strategy : we deeply optimize logic but we may break its
    // nice structure than can map in nice DFF enable 
    // (ex: big_designs/VexRiscv).
    // But it may help for some designs (ex: medium_designs/xtea)
    //
    run("opt -full");

    run("pmux2shiftx");

    run("techmap -map +/techmap.v");

    // BRAM inference 
    //
    infer_BRAMs();

    // After doing memory mapping, turn any remaining
    // $mem_v2 instances into flop arrays
    //
    run("memory_map");

    run("demuxmap");
    run("simplemap");

    // Call the zero asic version of 'opt_dff', e.g 'zopt_dff', especially 
    // taking care of the -sat option.
    //
    if (!no_opt_sat_dff) {

      run("stat");

      if (!no_opt_const_dff) {
        run("zopt_const_dff");
      }

      run("zopt_dff -sat");

      if (!no_opt_const_dff) {
        run("zopt_const_dff");
      }

    } else {

      if (!no_opt_const_dff) {
        run("zopt_const_dff");
      }
    }

    // Extra lines that help to win Area (ex: vga_lcd from 31K Lut4 downto 14.8K)
    //
    // IMPROVE-2
    //
    run("techmap");

#if 0
    run("opt -fast");

    run("opt_clean");
    // END IMPROVE-2
#endif

    // Performs 'opt' pass with lightweight version for HUGE designs.
    //
    if (getNumberOfCells() <= HUGE_NB_CELLS) {
       run("opt -full");
    } else {
       run("opt_expr");
       run("opt_clean");
    }

    // original TCL call : legalize_flops $sc_syn_feature_set
    //
    legalize_flops (); // C++ version of TCL call

    // C++ Version
    //
    // Map on the DFF of the architecture (partname)
    //
    string sc_syn_flop_library = ys_dff_techmap;
    run("techmap -map " + sc_syn_flop_library);

    // 'post_techmap' without arguments gives the following 
    // according to '.../siliconcompiler/tools/yosys/procs.tcl'
    // IMPROVE-1
    //
    run("techmap");

    // Performs 'opt' pass with lightweight version for HUGE designs.
    //
    if (getNumberOfCells() <= HUGE_NB_CELLS) {
       run("opt -purge");
    } else {
       run("opt_expr");
       run("opt_clean");
    }
    // END IMPROVE-1

    // Perform preliminary buffer insertion before passing to ABC to help reduce
    // the overhead of final buffer insertion downstream
    //
    if (insbuf) {
      run("insbuf");
    }

    run("stat");

    dbg_wait();

    // Binarize long XOR tree chains in delay mode. Typical example is 'gray2bin'
    // in 'logikbench/basic' suite having 64 levels that can be reduced to 6 levels.
    //
    if (!no_xor_tree_process && (opt == "delay")) {

      binary_decomp_xor_trees(topModule);
    }

    run("stat");

    // Optimize and map through ABC the combinational logic part of the design.
    //
    abc_synthesize();

    run("stat");

    dbg_wait();

    run("splitcells");

    run("splitnets");

    clean_design(true);

    dbg_wait();

    run("opt_lut_ins");
    run("opt_lut");

    run("setundef -zero");
    run("clean -purge");

    analyze_undriven_nets(topModule, true /* connect undriven nets to undef */);

    // tries to give public names instead of using $abc generic names.
    // Right now this procedure blows up runtime for medium/big designs.
    //
    if (autoname) {
      run("autoname");
    }

    if (!verilog_file.empty()) {

       log("Dump Verilog file '%s'\n", verilog_file.c_str()); 
       run(stringf("write_verilog -noexpr -nohex -nodec %s", verilog_file.c_str()));

    } else { // Still dump verilog under the hood for debug/analysis reasons.

       run(stringf("write_verilog -noexpr -nohex -nodec %s", "netlist_synth_fpga.verilog"));
    }

    run("stat");

    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

    float totalTime = 1 + elapsed.count() * 1e-9;

    log("   PartName   : %s\n", part_name.c_str());
    log("   DSP Style  : %s\n", dsp_tech.c_str());
    log("   BRAM Style : %s\n", bram_tech.c_str());
    log("\n");
    log("   'Zero Asic' FPGA Synthesis Version : %s\n", SYNTH_FPGA_VERSION);
    log("\n");
    log("   Total Synthesis Run Time = %.1f sec.\n", totalTime);

    // Show longest path in 'delay' mode
    //
    if (show_max_level) {
      run("max_level"); // -> store 'maxlvl' in scratchpad with 'max_level.max_levels'
    }

    if (csv) {
       dump_csv_file("stat.csv", (int)totalTime);
    }

    log("\n");
    log("***********************************\n");
    log("** Zero Asic FPGA Synthesis Done **\n");
    log("***********************************\n");

  } // end script()

} SynthFpgaPass;

PRIVATE_NAMESPACE_END
