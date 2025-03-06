#!/usr/bin/env node

import { program } from "commander";
import { embed } from "./commands/embed.js";
import { start } from "./commands/start.js";

program.command("embed")
    .description("Generate embedding headers for given files")
    .argument("<source>", "Directory to embed")
    .argument("[destination]", "Directory to write embed files to", "embedded")
    .action(embed);

program.command("start")
    .description("Start a development server and forward local address to given executable")
    .argument("<command>", "The command used to start the development server")
    .argument("<executable>", "The executable to invoke")
    .argument("[args...]", "Additional arguments to pass to the executable")
    .action(start);

program.parse();
