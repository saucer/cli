import { readdir } from "fs/promises";
import { resolve, relative } from "path";

export async function* recursiveDirectoryIterator(path: string, root: string = path): AsyncGenerator<{absolute: string, relative: string}>
{
    const entries = await readdir(path, { withFileTypes: true });

    for (const entry of entries)
    {
        const rtn = resolve(path, entry.name);

        if (entry.isDirectory())
        {
            yield* recursiveDirectoryIterator(rtn, root);
        }
        else
        {
            yield { absolute: rtn, relative: relative(root, rtn) };
        }
    }
}
