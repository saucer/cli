import figureSet from "figures";
import { Box, BoxProps, Text, TextProps } from "ink";
import { ReactNode } from "react";
import theme from "../theme/index.js";

function unique<T>(array: T[])
{
    return [...new Set(array)];
}

interface TableProps extends BoxProps
{
    data: any[];
    distribution: number[];
    colors?: TextProps["color"][];
}

export function Table({ data, distribution, colors, ...props }: TableProps)
{
    const headers = unique(data.map(item => Object.keys(item)).flat());
    const width = (index: number) => `${distribution[index]}%`;

    const border = props?.borderStyle ?? {
        top        : "-",
        bottom     : "-",
        left       : "|",
        right      : "|",
        bottomLeft : figureSet.lineUpRight,
        bottomRight: figureSet.lineUpLeft,
        topLeft    : figureSet.lineDownRight,
        topRight   : figureSet.lineDownLeft,
    };

    const style =
    {
        borderStyle: border,
        borderTop  : false,
        borderRight: false,
        borderLeft : false,
    };

    const row_style = (index: number) =>
    {
        const rtn = { ...style, borderBottom: true };

        if (index === (data.length - 1))
        {
            rtn.borderBottom = false;
        }

        return rtn;
    };

    const cell_style = (index: number) =>
    {
        const rtn = { ...style, borderBottom: false };

        if (headers.length > 1 && index < headers.length - 1)
        {
            rtn.borderRight = true;
        }

        return rtn;
    };

    const Cell = ({ index, children }: { index: number, children: ReactNode}) =>
        <Box
            marginX={1}
            width={width(index)}
            {...cell_style(index)}
        >
            {children}
        </Box>;

    return <Box
        flexDirection="column"
        borderStyle={border}
        {...props}
    >
        <Box {...style}>
            {headers.map((header, index) =>
                <Cell
                    key={header}
                    index={index}
                >
                    <Text
                        color={theme.colors.purple}
                        bold
                    >
                        {header}
                    </Text>
                </Cell>
            )}
        </Box>

        {data.map((row, key) =>
            <Box
                key={key}
                {...row_style(key)}
            >
                {headers.map((header, index) =>
                    <Cell
                        index={index}
                        key={`${header}-${key}`}
                    >
                        <Text color={colors?.[index]}>
                            {row[header]}
                        </Text>
                    </Cell>
                )}
            </Box>
        )}
    </Box>;
}
