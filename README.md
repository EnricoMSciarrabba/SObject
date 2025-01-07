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


### Automatic Connection Management During the Destruction of an `SObject`
When an `SObject` is destroyed, the system automatically removes all connections to this object. In practice, each time an object is destroyed:
1. All connections emitted by it are removed.
2. All connections received by it are removed.
3. Memory is cleaned up by removing any slots no longer associated with signals.
This prevents potential memory leaks and keeps the system clean.


## Main Structures and Classes


### SObject
The SObject class is the basis for implementing the Signal and Slot mechanism. Any object that wants to use this communication system must inherit from SObject. This class manages signals and connections between signals and slots, allowing communication between objects in a decoupled way.


#### Members Accessible to the User

Public Members:
- Constructor and Destructor:
  - SObject(): Default constructor. Allows the creation of derived objects.
  - ~SObject(): Destroys the object and releases all connections (associated signals and slots). Removes all associations with signals and slots of other objects.
- Methods to Verify Connection Status:
- signalIsPresent:
  - Checks whether a specific signal is present in the signal map (m_signalToSlotMap).
  - Useful for verifying if a signal is connected to any slot.
  ```cpp
  bool signalIsPresent(_sobject::_SignalBase* signal) const:
  ```
- connectedWithObject:
  - Verifies if the current object is connected to another object through at least one signal.
  ```cpp
  bool connectedWithObject(SObject* receiver) const:
  ```
- getAllReceivers:
  - Returns a list of all receivers (other connected objects).
  - If a specific signal is provided as a parameter, it returns only the receivers associated with that signal.
  ```cpp
  std::list<SObject*> getAllReceivers(_sobject::_SignalBase* signal = nullptr) const:
  ```
  
Protected Members:
- emitSignal:
  - Enables emitting a signal.
  - Invokes all slots associated with the specific signal, passing the provided parameters.
  - This method is protected to ensure that only derived classes can emit signals.
  ```cpp
  template <typename SM, typename... Args> void emitSignal(SM signalMethod, Args&&... args) const:
  ```


#### Internal Members (Not Usable Directly by the User)

Private Members:
Data Structures for Managing Connections:
- m_signalToSlotMap:
  - A map associating signals (_SignalBase) with lists of slots (_SlotBase).
  - Manages connections between the signals of the object and the methods (slots) of recipient objects.
    ```cpp
    std::map<const _sobject::_SignalBase*, std::list<_sobject::_SlotBase*>> m_signalToSlotMap:
    ```
- m_slotToSignalObjectList:
  - A list that tracks objects connected to the current object through slots.
  ```cpp
  std::list<SObject*> m_slotToSignalObjectList:
  ```
 
Private Methods for Managing Connections:
- removeSignalSlotConnection:
  - Removes a connection between a specific signal and a specific slot.
  ```cpp
  void removeSignalSlotConnection(_sobject::_SignalBase* signal, _sobject::_SlotBase* slot):
  ```
- removeSignalReceiverSlotConnection:
  - Removes all connections of a specific signal to a specific receiver object.
  ```cpp
  void removeSignalReceiverSlotConnection(_sobject::_SignalBase* signal, SObject* receiver):
  ```
- removeAllSignalSlotConnection:
  - Removes all connections for a specific signal.
  ```cpp
  void removeAllSignalSlotConnection(_sobject::_SignalBase* signal):
  ```
- removeAllConnection:
  - Removes all connections of an object (signals and associated slots).
  ```cpp
  void removeAllConnection():
  ```

| **Category**              | **Members**                                                                                                    | **Description**                                                                                                          | **Access**     |
|---------------------------|---------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------|----------------|
| **Constructors/Destructors** | `SObject()`, `~SObject()`                                                                                    | Constructs or destroys an `SObject`. Removes all connections when destroyed.                                             | **Public**     |
| **Connection Management** | `signalIsPresent`, `connectedWithObject`, `getAllReceivers`                                                   | Methods to verify and retrieve connections between signals and objects.                                                  | **Public**     |
| **Signal Emission**       | `emitSignal`                                                                                                  | Allows emitting signals to notify events. Usable only by derived classes.                                                | **Protected**  |
| **Internal Structures**   | `m_signalToSlotMap`, `m_slotToSignalObjectList`                                                               | Internal data structures for tracking signals and connections.                                                           | **Private**    |
| **Internal Management**   | `removeSignalSlotConnection`, `removeSignalReceiverSlotConnection`, `removeAllSignalSlotConnection`, `removeAllConnection` | Private methods for managing the removal of connections (signals and slots). Not intended for direct user interaction. | **Private**    |



### The _sobject namespace
The _sobject namespace defines a system for handling signals and slots, a mechanism commonly used in frameworks like Qt to implement communication between objects. This code is intended to be used by other classes and structures that implement object communication logic, but it should not be used directly by the user. Here’s a simple explanation of its contents and functionality:


#### Signal System (_Signal)
Signals represent events generated by an object (emitter) that can be connected to functions (slots) of other objects.
Main Components:
- _SignalBase: Base class for all signals. It includes:
  - A pointer to the emitting object (m_emitter).
  - Functions to compare two signals.
- _Signal: A template class for defining specific signals.
  - Associates a signal with a method of an object.
  - Allows comparison between signals (same object and method).


#### Slot System (_Slot)
Slots are methods that can be called when a signal is emitted.
Main Components:
- _SlotBase: Base class for all slots. It includes:
  - Virtual methods to compare slots, execute a slot, and manage input parameters.
  - Storage for input parameters (m_inputParams).
- _Slot: A template class for specific slots.
  - Contains a pointer to the receiving method (m_slot) and the receiver (m_receiver).
  - Can compare slots, verify the receiver, and execute the method with provided parameters.


#### Method Invocation:
To execute a method associated with a slot:
- invokeSlot: A generic function to call an object’s method using parameters passed as std::any.


#### Custom Comparators:
The namespace includes structures for comparing and managing signals and slots:
- CustomSignalCompare: Compares signals, checks if they are equal, and deletes the associated object if needed.
- CustomSlotCompare: Compares slots or receivers, checks if they are equal, and deletes the slot if necessary.


#### Why not use the _sobject namespace directly?
The namespace contains internal implementations, designed to be used by higher-level structures.
It is not designed for direct use, as it lacks the tools and safeguards needed to prevent errors.
Direct use requires an in-depth understanding of its components and their functionality.



















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
