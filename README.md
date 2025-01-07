# SObject - Signal and Slot Framework in C++


## Description


SObject is a C++ library designed to replicate the Signal and Slot mechanism, similar to the one available in frameworks like Qt. This implementation enables the connection of signals to slots, allowing for asynchronous and reactive communication between objects. By using this library, developers can design modular and flexible systems, ideal for GUI applications, event-driven systems, or any scenario requiring a publisher-subscriber communication paradigm.


## Main Features

1. **Connect**: Allows connecting a signal emitted by one object (`emitter`) to a slot of another object (`receiver`).
2. **Disconnect**: Allows removing a connection between a signal and a slot.
3. **Automatic connection management**: When an `SObject` is destroyed, all its connections are automatically removed, preventing memory leaks and keeping the system clean.


### How the Connect System Works
The system is based on a connection between two main components:
- **Signal**: Represents a signal emitted by an object.
- **Slot**: Represents a function that reacts to the signal, i.e., a callback associated with the signal.
When an object emits a signal, the slots connected to that signal are executed.


### The `connect` Function
The `connect` function establishes a connection between a signal emitted by an object (`emitter`) and a slot of another object (`receiver`). The connection is stable until explicitly disconnected.


### Syntax
```cpp
connect(emitter, &EmitterClass::signalMethod, receiver, &ReceiverClass::slotMethod);
```
- `emitter`: Object that emits the signal.
- `signalMethod`: Member method of `emitter` that represents the signal.
- `receiver`: Object that receives the signal and whose slot should be executed.
- `slotMethod`: Member method of `receiver` that represents the slot.


### How the `connect` Function Works:
1. A temporary object is created to identify the signal and the slot.
2. The connection is added to the emitter's signal map.
3. If no similar connection exists, a new record is created.
4. The emitter is registered in the receiver's list, ensuring that the receiver can "listen" to the emitter's signals.


### The `disconnect` Function
The `disconnect` function removes a connection between a signal and a slot. There are various methods to disconnect:


### Syntax to disconnect a specific slot
```cpp
disconnect(emitter, &EmitterClass::signalMethod, receiver, &ReceiverClass::slotMethod);
```


### Syntax to disconnect all slots of a receiver for a specific signal
```cpp
disconnect(emitter, &EmitterClass::signalMethod, receiver);
```


### Syntax to disconnect all slots for a signal
```cpp
disconnect(emitter, &EmitterClass::signalMethod);
```


### Syntax to disconnect all connections of an emitter
```cpp
disconnect(emitter);
```


### How the `disconnect` Function Works:
1. **Single disconnection**: The function removes a specific connection between a signal and a slot. After removing the connection, if there are no other slots for that signal, the signal is deleted from the emitter's map.
2. **Disconnection by receiver**: Disconnects all slots of a receiver for a specific signal.
3. **Disconnection by signal**: Removes all connections between a signal and its associated slots.
4. **Global disconnection**: Removes all connections of an emitter (signals and associated slots).


### Automatic Connection Management During the Destruction of an `SObject`
When an `SObject` is destroyed, the system automatically removes all connections to this object. In practice, each time an object is destroyed:
1. All connections emitted by it are removed.
2. All connections received by it are removed.
3. Memory is cleaned up by removing any slots no longer associated with signals.
This prevents potential memory leaks and keeps the system clean.


## Main Structures and Classes


### `SObject`
The base class that represents an object capable of emitting signals and receiving slots. It can be connected to another object through the `connect` function and can disconnect via the `disconnect` function.
- **Main functions**:
  - `emitSignal`: Emits a signal.
  - `getAllReceivers`: Returns all receivers associated with a signal.
  - `signalIsPresent`: Checks if a signal is associated with any slots.
  - `connectedWithObject`: Checks if the object is connected to another.


## Example


### Code
```cpp
#include <sobject.h>

class Emitter : public SObject
{
public:
    S_SIGNAL void signalA(int x){};
    S_SIGNAL void signalB(int x){};
    
    void doSomethingA(int x)
    {
        std::cout << "Emit SignalA: " << x << std::endl;
        emitSignal(&Emitter::signalA, x);
    }
    
    void doSomethingB(int x)
    {
        std::cout << "Emit SignalB: " << x << std::endl;
        emitSignal(&Emitter::signalB, x);
    }
};

class Receiver : public SObject
{
public:
    S_SLOT void slotA(int x)
    {
        std::cout << "Receiver: slotA(" << x << ") received!" << std::endl;
    }
    
    S_SLOT void slotB(int x)
    {
        std::cout << "Receiver: slotB(" << x << ") received!" << std::endl;
    }
};

int main()
{
    Emitter* emitter = new Emitter();
    Receiver* receiver = new Receiver();

    connect(emitter, &Emitter::signalA, receiver, &Receiver::slotA);
    connect(emitter, &Emitter::signalA, receiver, &Receiver::slotB);

    emitter->doSomethingA(42);
    std::cout << std::endl;

    disconnect(emitter, &Emitter::signalA, receiver);

    emitter->doSomethingA(100);
    std::cout << std::endl;
    
    connect(emitter, &Emitter::signalA, receiver, &Receiver::slotB);
    connect(emitter, &Emitter::signalB, receiver, &Receiver::slotA);
    
    emitter->doSomethingB(100);
    std::cout << std::endl;

    delete receiver;

    emitter->doSomethingB(50);
    std::cout << std::endl;

    delete emitter;

    return 0;
}
```


### Result
```yaml
Emit SignalA: 42
Receiver: slotA(42) received!
Receiver: slotB(42) received!

Emit SignalA: 100

Emit SignalB: 100
Receiver: slotA(100) received!

Emit SignalB: 50
```


## Conclusion

This *Signal-Slot* system allows elegant and flexible communication between objects in C++. The automatic management of connections during object destruction avoids errors related to manual resource management and simplifies event-driven programming.
