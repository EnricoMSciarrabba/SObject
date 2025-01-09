# SObject - Signal and Slot Framework in C++

This project provides a simple implementation of Qt's signal-slot mechanism in C++11. The SObject class is designed to be easily integrated into any C++11 project.

## Key Features

1: **Inheritance from SObject:** Simply inherit from the SObject class to use signals and slots in a Qt-like manner. The emitSignal function allows emitting signals with specific parameters and automatically     triggers connected slots.
  ```cpp
    template <typename Emitter, typename... Args>
    void emitSignal(void(Emitter::* const signalM)(Args...), Args... args) const
  ```

2: **Connect and Disconnect methods:** Signals and slots can be connected and disconnected using global methods.They allow connecting and disconnecting signals to slots for specific emitter and receiver objects, with overloads for different use cases.

  ```cpp
    template<typename Emitter, typename Receiver, typename... Args>
    void connect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver, void(Receiver::*slotM)(Args...))
  ```
  ```cpp
    template<typename Emitter, typename Receiver, typename... Args>
    void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver, void(Receiver::*slotM)(Args...))
  ```
  ```cpp
    template<typename Emitter, typename Receiver, typename... Args>
    void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver)
  ```
  ```cpp
    template<typename Emitter, typename... Args>
    void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...))
  ```
  ```cpp
    void disconnect(SObject* emitter)
  ```

3: **Automatic disconnection management:** When an SObject is destroyed, all associated signal-slot connections are automatically disconnected to prevent memory leaks or crashes.

## How to Use

1: Inherit from SObject in your class.
2: Use connect to connect signals to slots and disconnect to disconnect them.
3: Use emitSignal to emit signals and trigger slots.

## Example

### Code

```cpp
#include <iostream>
#include <sobject.h>

class A : public SObject
{
public:
    void signal(int){};
    void send(int num)
    {
        emitSignal(&A::signal, num);
    };
};

class B : public SObject
{
public:
    S_SLOT void slot(int num){std::cout << "B " << num << std::endl;};
};

class C : public SObject
{
public:
    S_SLOT void slot(int num){std::cout << "C " << num << std::endl;};
};

int main()
{
    A a;
    A* aa = new A;
    {
        B b;

        connect(&a, &A::signal, &b, &B::slot);
        a.send(4);

        connect(&a, &A::signal, &b, &B::slot);
        a.send(5);

        connect(aa, &A::signal, &b, &B::slot);
        a.send(8);
        aa->send(9);

        disconnect(&a, &A::signal, &b);
        a.send(2);
        aa->send(10);

        connect(&a, &A::signal, &b, &B::slot);
        a.send(3);
        aa->send(4);
    }

    C c;
    connect(&a, &A::signal, &c, &C::slot);

    a.send(5);
    aa->send(6);

    return 0;
}
```

### Result

```yaml
B 4
B 5
B 5
B 8
B 8
B 9
B 10
B 3
B 4
C 5
```

## Conclusion

This *Signal-Slot* system allows elegant and flexible communication between objects in C++. The automatic management of connections during object destruction avoids errors related to manual resource management and simplifies event-driven programming.
