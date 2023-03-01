
import argparse
import json
import copy
import os


parser = argparse.ArgumentParser()
parser.add_argument('--out_folder', required=True)
parser.add_argument('--config', required=True)
parser.add_argument('--seed', required=True)

args = parser.parse_args()

cfg_base = json.load(open(args.config))

cfg_file_name = os.path.basename(args.config)

ALL_SCHEMES = [ "ReedSolomonMultiFrame", "StreamingCode" ]

for coding_scheme in cfg_base["coding_schemes"]:
    for tau in cfg_base[coding_scheme]["taus"]:
        new_cfg = copy.deepcopy(cfg_base)

        new_cfg.pop("coding_schemes", None)
        new_cfg["coding_scheme"] = coding_scheme
        new_cfg["tau"] = tau
        new_cfg["seed"] = int(args.seed)

        for sc in ALL_SCHEMES:
            if sc != coding_scheme:
                new_cfg.pop(sc, None)

        output_file = os.path.join(args.out_folder, cfg_file_name + "." + coding_scheme + "_" + str(tau))

        if "models" in cfg_base[coding_scheme]:
            for model, model_mean, model_var in zip(cfg_base[coding_scheme]["models"], cfg_base[coding_scheme]["model_means"], cfg_base[coding_scheme]["model_vars"]):
                new_cfg[coding_scheme].pop("models", None)
                new_cfg[coding_scheme]["model"] = model
                new_cfg[coding_scheme]["model_means"] = model_mean
                new_cfg[coding_scheme]["model_vars"] = model_var

                out = output_file + "_" + os.path.basename(model)
                with open(out, 'w') as wf:
                    json.dump(new_cfg, wf, indent=4)

        else:
            with open(output_file, 'w') as wf:
                json.dump(new_cfg, wf, indent=4)

