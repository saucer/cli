export class Expected<R, E>
{
    private m_value: R | undefined;
    private m_error: E | undefined;

    private constructor(value?: R, error?: E)
    {
        this.m_value = value;
        this.m_error = error;
    }

    static expected<R, E>(value: R): Expected<R, E>
    {
        return new Expected<R, E>(value);
    }

    static unexpected<R, E>(error: E): Expected<R, E>
    {
        return new Expected<R, E>(undefined, error);
    }

    public success(): boolean
    {
        return typeof this.m_value !== "undefined";
    }

    public value(): R
    {
        return this.m_value!;
    }

    public error(): E
    {
        return this.m_error!;
    }
}
