import { ChildProcess, exec, spawn } from "child_process";

const host_regex = /https?:\/\/(localhost|0\.0\.0\.0)\:(\d+)/g;

export async function start(command: string, executable: string, args: string[])
{
    const command_child = exec(command);
    let executable_child: ChildProcess | undefined;

    command_child.stdout?.on("data", (data: string) =>
    {
        const match = data.match(host_regex);

        if (!match)
        {
            return;
        }

        executable_child = spawn(executable, [match[0], ...args]);

        executable_child.stdout?.pipe(process.stdout);
        executable_child.stderr?.pipe(process.stderr);
    });

    command_child.stdout?.pipe(process.stdout);
    command_child.stderr?.pipe(process.stderr);

    await new Promise(resolve => command_child.on("exit", resolve));
    await new Promise(resolve => executable_child?.on("exit", resolve));
}
