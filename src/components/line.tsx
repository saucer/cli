import { Text, TextProps } from "ink";

export type ColoredText = string | [text: string, color: TextProps["color"]];

interface LineProps
{
    icon: React.JSX.Element;
    text: string | ColoredText[];
}

export function TextItem({ item }: {item: ColoredText})
{
    if (typeof item === "string")
    {
        return <Text>{item}</Text>;
    }

    return <Text color={item[1]}>{item[0]}</Text>;
}

export function Line({ icon, text }: LineProps)
{
    return <Text>
        {icon}
        {" "}
        {
            Array.isArray(text) ?
                text.map((item, index) => <TextItem key={index} item={item} />)
                :
                <TextItem item={text} />
        }
    </Text>;
}
