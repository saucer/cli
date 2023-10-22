import figureSet from "figures";
import { existsSync, lstatSync, readFileSync, statSync } from "fs";
import { outputFile } from "fs-extra";
import { mkdir } from "fs/promises";
import { Newline, Text, render } from "ink";
import Spinner from "ink-spinner";
import mimes from "mime-types";
import { resolve } from "path";
import { Fragment, ReactNode } from "react";
import { Table } from "./components/table.js";
import colors from "./utils/colors.js";
import { Expected } from "./utils/expected.js";
import { recursiveDirectoryIterator } from "./utils/fs.js";

export class File
{
    private m_stream: Buffer;

    private m_name: string;
    private m_mime: string;

    private constructor(stream: Buffer, name: string, mime: string)
    {
        this.m_stream = stream;

        this.m_name = name;
        this.m_mime = mime;
    }

    static from(absolute: string, relative: string): Expected<File, string>
    {
        if (!existsSync(absolute))
        {
            return Expected.unexpected("File not found");
        }

        if (!lstatSync(absolute).isFile())
        {
            return Expected.unexpected("Not a file");
        }

        const mime = mimes.lookup(relative);

        if (!mime)
        {
            return Expected.unexpected("Could not determine mime");
        }

        const stream = readFileSync(absolute);
        return Expected.expected(new File(stream, relative, mime));
    }

    public async embed(target: string)
    {
        await outputFile(target,
            `#pragma once
#include <array>
#include <cstdint>

namespace saucer::embedded
{
    inline constexpr std::array<std::uint8_t, ${this.m_stream.byteLength}> ${this.escaped_name()} = {
        ${[...this.m_stream].map(b => `0x${b.toString(16)}`).join(", ")}
    };
} // namespace saucer::embedded
`
        );
    }

    public name()
    {
        return this.m_name;
    }

    public escaped_name()
    {
        return this.m_name.replaceAll(/[^a-z]/ig, "_");
    }

    public mime()
    {
        return this.m_mime;
    }
}

export async function embed(source: string, destination: string)
{
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
        const result = File.from(absolute, relative);

        if (!result.success())
        {
            errors.push(
                <Text>
                    <Text color="red">{figureSet.cross}</Text>
                    {" Failed to embed "}
                    <Text color="redBright">{relative}</Text>
                    {`: ${result.error()}`}
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

        const file = result.value();
        const target = resolve(destination, `${relative}.hpp`);

        try
        {
            await file.embed(target);
        }
        catch (error)
        {
            errors.push(
                <Text>
                    <Text color="red">{figureSet.cross}</Text>
                    {" Failed to write "}
                    <Text color={colors.purple}>{target}</Text>
                    {`: ${error}`}
                </Text>
            );

            continue;
        }

        files.push([target, file]);
    }

    try
    {
        await outputFile(resolve(destination, "all.hpp"),
            `#pragma once
#include <map>
#include <string>
#include <saucer/webview.hpp>

${files.map(([, file]) => `#include "${file.name()}.hpp"`).join("\n")}

namespace saucer::embedded
{
    inline auto all()
    {
        std::map<std::string, embedded_file> rtn;

        ${files.map(([, file]) => `rtn.emplace("${file.name()}", embedded_file{"${file.mime()}", ${file.escaped_name()}});`).join("\n        ")}

        return rtn;
    }
} // namespace saucer::embedded
`
        );
    }
    catch (error)
    {
        errors.push(
            <Text>
                <Text color="red">{figureSet.cross}</Text>
                {" Failed to write "}
                <Text color={colors.purple}>all.hpp</Text>
                {`: ${error}`}
            </Text>
        );
    }
    const table_data = files.map(([path, file]) =>
    {
        return {
            File       : file.name(),
            Mime       : file.mime(),
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
