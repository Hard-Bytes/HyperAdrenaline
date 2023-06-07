#pragma once 

struct State
{
    virtual void init()     = 0;
    virtual void resume()   = 0;
    virtual void update()   = 0;
    virtual void close()    = 0;
    virtual bool isAlive()  = 0;
    virtual ~State() = default;

    bool isTransparent {false};
};
