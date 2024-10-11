import { Eta } from "eta";
import { existsSync, lstatSync, statSync } from "fs";
import extra from "fs-extra";
import { mkdir } from "fs/promises";
import { fromPromise, fromThrowable } from "neverthrow";
import { dirname, join, resolve } from "path";
import { exit } from "process";
import { fileURLToPath } from "url";
import { File, parse } from "../file.js";
import { recursiveDirectoryIterator } from "../utils/fs.js";
import ora from "ora";

const { outputFileSync } = extra;

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

export async function embed(source: string, destination: string)
{
    const eta = new Eta({ views: join(__dirname, "..", "templates") });
    const as_str = (error: unknown) => error as string;

    if (!existsSync(destination))
    {
        const result = await fromPromise(mkdir(destination, { recursive: true }), as_str);

        if (result.isErr())
        {
            console.error(`Failed to create destination: ${destination} (${result.error})`);
            return exit(1);
        }
    }

    if (!existsSync(source) || !lstatSync(source).isDirectory())
    {
        console.error(`Expected '${source}' to be a directory`);
        return exit(1);
    }

    const files: [string, File][] = [];

    for await (const { absolute, relative } of recursiveDirectoryIterator(source))
    {
        const spinner = ora({ color: "magenta", text: `Embedding '${relative}'...` }).start();
        const file = parse(absolute, relative);

        if (file.isErr())
        {
            spinner.fail(`Failed to parse: ${relative} (${file.error})`);
            continue;
        }

        const target = resolve(destination, `${relative}.hpp`);
        const writeFile = fromThrowable(outputFileSync, as_str);

        const content = eta.render("embed", {
            name: file.value.name,
            data: file.value.data,
            size: file.value.size,
        });

        const result = writeFile(target, content);

        if (result.isErr())
        {
            spinner.fail(`Failed to write: ${relative} (${result.error})`);
            continue;
        }

        spinner.succeed(`Embedded '${relative}'`);
        files.push([target, file.value]);
    }

    const table = files.map(([path, file]) => ({
        File: file.path,
        Mime: file.mime,
        Header: path,
        "Size (KB)": (statSync(path).size / 1024),
    }));

    if (table.length === 0)
    {
        return;
    }

    console.log();
    console.table(table, ["File", "Mime", "Header", "Size (KB)"]);
}
