import figureSet from "figures";
import { Text } from "ink";
import { ColoredText, Line } from "./line.js";

interface ErrorProps
{
    error?: string | ColoredText[];
    description: string | ColoredText[];
}

export function Error({ description, error }: ErrorProps)
{
    return <Line
        icon={<Text color="red">{figureSet.cross}</Text>}
        text={[...description, ...(error ? [": ", ...error] : [])]}
    />;
}
