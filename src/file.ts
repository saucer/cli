import { exists } from "fs-extra";
import { lstat, readFile } from "fs/promises";
import mimes from "mime-types";
import { err, fromPromise, ok, Result } from "neverthrow";

export interface File
{
    path: string;
    mime: string;
    name: string;
    data: string;
    size: number;
}

export async function parse(absolute: string, relative: string): Promise<Result<File, string>>
{
    if (!await exists(absolute))
    {
        return err("file does not exist");
    }

    if (!(await lstat(absolute)).isFile())
    {
        return err("not a file");
    }

    const mime = mimes.lookup(relative);

    if (!mime)
    {
        return err("could not determine mime");
    }

    const stream = await fromPromise(readFile(absolute), err => err as string);

    if (!stream.isOk())
    {
        return err(stream.error);
    }

    const path = relative.replace(/\\/g, "/");
    const name = relative.replace(/[^a-z]/ig, "_");

    const size = stream.value.byteLength;
    const data = [...stream.value].map(b => `0x${b.toString(16)}`).join(", ");

    return ok({ path, mime, name, data, size });
}
