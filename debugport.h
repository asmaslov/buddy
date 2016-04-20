class DebugPort
{
  public:
    DebugPort();
    ~DebugPort();
    void printf(char *str, ...);
    void putchar(char c);
};
