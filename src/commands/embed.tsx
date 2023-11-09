import { Eta } from "eta";
import figureSet from "figures";
import { existsSync, lstatSync, statSync } from "fs";
import { outputFileSync } from "fs-extra/esm";
import { mkdir } from "fs/promises";
import { Newline, Text, render } from "ink";
import Spinner from "ink-spinner";
import { fromPromise, fromThrowable } from "neverthrow";
import path, { dirname, resolve } from "path";
import { Fragment, ReactNode } from "react";
import { Error } from "../components/error.js";
import { Table } from "../components/table.js";
import { File, parse } from "../file.js";
import theme from "../theme/index.js";
import { recursiveDirectoryIterator } from "../utils/fs.js";
import { fileURLToPath } from "url";
import { Line } from "../components/line.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

export async function embed(source: string, destination: string)
{
    const errors: ReactNode[] = [];
    const { unmount, rerender } = render(<Fragment />);

    const eta = new Eta({ views: path.join(__dirname, "..", "templates") });
    const writeFile = fromThrowable(outputFileSync, error => (error as string));

    if (!existsSync(destination))
    {
        const result = await fromPromise(mkdir(destination, { recursive: true }), error => (error as string));

        if (result.isErr())
        {
            rerender(
                <Error
                    description={["Failed to create destination (", [destination, "gray"], ")"]}
                    error={result.error}
                />
            );

            unmount(1);
            return;
        }
    }

    if (!existsSync(source) || !lstatSync(source).isDirectory())
    {
        rerender(
            <Error
                description={["Expected ", [source, "redBright"], " to be directory"]}
            />
        );

        unmount(1);
        return;
    }

    const files: [string, File][] = [];

    for await (const { absolute, relative } of recursiveDirectoryIterator(source))
    {
        const file = parse(absolute, relative);

        if (file.isErr())
        {
            errors.push(
                <Error
                    description={["Failed to embed ", [relative, "redBright"]]}
                    error={file.error}
                />
            );

            continue;
        }

        rerender(
            <>
                {...errors}
                <Line
                    icon={
                        <Text color={theme.colors.purple} dimColor>
                            <Spinner type="arc" />
                        </Text>
                    }
                    text={["Embedding ", [relative, "greenBright"]]}
                />
            </>
        );

        const target = resolve(destination, `${relative}.hpp`);
        const result = writeFile(target, eta.render("embed", { name: file.value.name, data: file.value.data, size: file.value.size }));

        if (result.isErr())
        {
            errors.push(
                <Error
                    description={["Failed to write ", [target, "redBright"]]}
                    error={result.error}
                />
            );

            continue;
        }

        files.push([target, file.value]);
    }

    const result = writeFile(resolve(destination, "all.hpp"), eta.render("all", { files: files.map(x => x[1]) }));

    if (result.isErr())
    {
        errors.push(
            <Error description="Failed to write" error={result.error} />
        );

        unmount(1);
    }

    const table_data = files.map(([path, file]) => ({
        File       : file.path,
        Mime       : file.mime,
        Header     : path,
        "Size (KB)": (statSync(path).size / 1024).toFixed(1),
    }));

    rerender(
        <>
            <Table
                data={table_data}
                distribution={[40, 15, 40, 5]}
                colors={[undefined, "gray", undefined, theme.colors.purple]}
            />
            <Newline />
            {...errors}
            <Line
                icon={<Text color="greenBright">{figureSet.tick}</Text>}
                text={["Embedded ", [files.length.toString(), theme.colors.purple], " file(s)\n"]}
            />
        </>
    );

    unmount(0);
}
