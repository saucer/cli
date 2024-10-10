import { existsSync, lstatSync, readFileSync } from "fs";
import mimes from "mime-types";
import { err, ok, Result } from "neverthrow";

export interface File
{
    path: string;
    mime: string;
    name: string;
    data: string;
    size: number;
}

export function parse(absolute: string, relative: string): Result<File, string>
{
    if (!existsSync(absolute))
    {
        return err("file does not exist");
    }

    if (!lstatSync(absolute).isFile())
    {
        return err("not a file");
    }

    const mime = mimes.lookup(relative);

    if (!mime)
    {
        return err("could not determine mime");
    }

    const stream = readFileSync(absolute);

    const path = relative.replace(/\\/g, "/");
    const name = relative.replace(/[^a-z]/ig, "_");

    const size = stream.byteLength;
    const data = [...stream].map(b => `0x${b.toString(16)}`).join(", ");

    return ok({ path, mime, name, data, size });
}
