#!/usr/bin/env node

import { program } from "commander";
import { embed } from "./embed.js";

program.command("embed")
    .description("Generate embedding headers for given files")
    .argument("<source>", "Directory to embed")
    .argument("[destination]", "Directory to write embed files to", "embedded")
    .action(embed);

program.parse();
