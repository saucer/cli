import { Eta } from "eta";
import figureSet from "figures";
import { existsSync, lstatSync, statSync } from "fs";
import { outputFileSync } from "fs-extra/esm";
import { mkdir } from "fs/promises";
import { Newline, Text, render } from "ink";
import Spinner from "ink-spinner";
import { Result } from "neverthrow";
import path, { resolve } from "path";
import { Fragment, ReactNode } from "react";
import { Table } from "./components/table.js";
import { File, parse } from "./file.js";
import colors from "./utils/colors.js";
import { recursiveDirectoryIterator } from "./utils/fs.js";

export async function embed(source: string, destination: string)
{
    const writeFile = Result.fromThrowable(outputFileSync);
    const eta = new Eta({ views: path.join(new URL(path.dirname(import.meta.url)).pathname, "templates") });

    const { unmount, rerender } = render(<Fragment />);
    const errors: ReactNode[] = [];

    if (!existsSync(destination))
    {
        try
        {
            await mkdir(destination, { recursive: true });
        }
        catch (error)
        {
            rerender(
                <Text>
                    <Text color="red">{figureSet.cross}</Text>
                    {" Failed to create destination ("}
                    <Text color="gray">{destination}</Text>
                    {`): ${error}`}
                </Text>
            );

            unmount(1);
            return;
        }
    }

    if (!existsSync(source) || !lstatSync(source).isDirectory())
    {
        rerender(
            <Text>
                <Text color="red">{figureSet.cross}</Text>
                {" Expected "}
                <Text color="redBright">{source}</Text>
                {" to be directory"}
            </Text>
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
                <Text>
                    <Text color="red">{figureSet.cross}</Text>
                    {" Failed to embed "}
                    <Text color="redBright">{relative}</Text>
                    {`: ${file.error}`}
                </Text>
            );

            continue;
        }

        rerender(
            <>
                {...errors}
                <Text>
                    <Text color={colors.purple} dimColor>
                        <Spinner type="arc" />
                    </Text>
                    {" Embedding "}
                    <Text color="greenBright">
                        {relative}
                    </Text>
                </Text>
            </>
        );

        const target = resolve(destination, `${relative}.hpp`);
        const result = writeFile(target, eta.render("embed", { name: file.value.name, data: file.value.data, size: file.value.size }));

        if (result.isErr())
        {
            errors.push(
                <Text>
                    <Text color="red">{figureSet.cross}</Text>
                    {" Failed to write "}
                    <Text color={colors.purple}>{target}</Text>
                    {`: ${result.error}`}
                </Text>
            );

            continue;
        }

        files.push([target, file.value]);
    }

    const result = writeFile(resolve(destination, "all.hpp"), eta.render("all", { files: files.map(x => x[1]) }));

    if (result.isErr())
    {
        errors.push(
            <Text>
                <Text color="red">{figureSet.cross}</Text>
                {" Failed to write "}
                <Text color={colors.purple}>all.hpp</Text>
                {`: ${result.error}`}
            </Text>
        );

        unmount(1);
    }

    const table_data = files.map(([path, file]) =>
    {
        return {
            File       : file.path,
            Mime       : file.mime,
            Header     : path,
            "Size (KB)": (statSync(path).size / 1024).toFixed(1),
        };
    });

    rerender(
        <>
            <Table data={table_data} distribution={[40, 15, 40, 5]} color={[undefined, "gray", undefined, colors.purple]} />
            <Newline />
                {...errors}
                <Text>
                    <Text color="greenBright">{figureSet.tick}</Text>
                    {" Embedded "}
                    <Text color={colors.purple}>{files.length}</Text>
                    {" file(s)"}
                </Text>
        </>
    );

    unmount(0);
}
