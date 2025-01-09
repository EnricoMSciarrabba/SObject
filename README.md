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

// EventEmitter class that emits events
class EventEmitter : public SObject
{
public:
    // Signal that will be emitted when an event occurs
    S_SIGNAL void eventOccurred(int){};

    // Method to trigger an event
    void triggerEvent(int eventData)
    {
        std::cout << "Emitting event with data: " << eventData << std::endl;
        emitSignal(&EventEmitter::eventOccurred, eventData);
    };
};

// Listener A that handles events from EventEmitter
class EventListenerA : public SObject
{
public:
    // Slot that handles the event and prints the received data
    S_SLOT void handleEvent(int eventData)
    {
        std::cout << "Listener A received: " << eventData << std::endl;
    };
};

// Listener B that handles events from EventEmitter
class EventListenerB : public SObject
{
public:
    // Slot that handles the event and prints the received data
    S_SLOT void handleEvent(int eventData)
    {
        std::cout << "Listener B received: " << eventData << std::endl;
    };
};

int main()
{
    std::cout << "\n*** Test case 1: Simple connection and event emission ***\n";
    // Create an EventEmitter object
    EventEmitter emitter1;

    // Create a listener object
    EventListenerA listenerA;

    // Connect emitter1's event to listenerA's slot
    connect(&emitter1, &EventEmitter::eventOccurred, &listenerA, &EventListenerA::handleEvent);

    // Trigger event from emitter1
    emitter1.triggerEvent(10);  // Expect: Listener A received: 10

    // Disconnect emitter1 from listenerA
    disconnect(&emitter1, &EventEmitter::eventOccurred, &listenerA);

    // Trigger event after disconnecting
    emitter1.triggerEvent(20);  // Expect no output since listenerA is disconnected

    std::cout << "\n*** Test case 2: Dynamic connection with a new emitter ***\n";
    // Create a new EventEmitter object dynamically
    EventEmitter* emitter2 = new EventEmitter;

    // Create another listener object
    EventListenerB* listenerB = new EventListenerB;

    // Connect emitter2's event to listenerB's slot
    connect(emitter2, &EventEmitter::eventOccurred, listenerB, &EventListenerB::handleEvent);

    // Trigger event from emitter2
    emitter2->triggerEvent(30);  // Expect: Listener B received: 30

    // Destroy listenerB object
    delete listenerB;
    
    // Trigger event after disconnecting
    emitter2->triggerEvent(35);  // Expect no output since ListenerB destroyed

    std::cout << "\n*** Test case 3: Reconnect after disconnection ***\n";
    // Reconnect emitter1 to listenerA again
    connect(&emitter1, &EventEmitter::eventOccurred, &listenerA, &EventListenerA::handleEvent);

    // Trigger event after reconnecting
    emitter1.triggerEvent(40);  // Expect: Listener A received: 40

    std::cout << "\n*** Test case 4: Multiple connections to the same event ***\n";
    // Reconnect emitter1 to listenerA
    connect(&emitter1, &EventEmitter::eventOccurred, &listenerA, &EventListenerA::handleEvent);

    // Trigger event again with multiple connections
    emitter1.triggerEvent(50);  // Expect: Listener A received: 50 (twice, once per connection)

    // Disconnect all connections from emitter1
    disconnect(&emitter1, &EventEmitter::eventOccurred);

    std::cout << "\n*** Test case 5: Multiple listeners on different emitters ***\n";
    // Create a new listener object
    EventListenerB listenerB2;

    // Connect emitter1 to listenerB2
    connect(&emitter1, &EventEmitter::eventOccurred, &listenerB2, &EventListenerB::handleEvent);

    // Trigger event from emitter1
    emitter1.triggerEvent(60);  // Expect: Listener B received: 60

    // Create a new emitter object dynamically
    EventEmitter* emitter3 = new EventEmitter;

    // Connect emitter3 to listenerB2
    connect(emitter3, &EventEmitter::eventOccurred, &listenerB2, &EventListenerB::handleEvent);

    // Trigger event from emitter3
    emitter3->triggerEvent(70);  // Expect: Listener B received: 70

    // Cleanup: Destroy emitter3
    delete emitter3;

    return 0;
}
```

### Result

```yaml
*** Test case 1: Simple connection and event emission ***
Emitting event with data: 10
Listener A received: 10
Emitting event with data: 20

*** Test case 2: Dynamic connection with a new emitter ***
Emitting event with data: 30
Listener B received: 30
Emitting event with data: 35

*** Test case 3: Reconnect after disconnection ***
Emitting event with data: 40
Listener A received: 40

*** Test case 4: Multiple connections to the same event ***
Emitting event with data: 50
Listener A received: 50
Listener A received: 50

*** Test case 5: Multiple listeners on different emitters ***
Emitting event with data: 60
Listener B received: 60
Emitting event with data: 70
Listener B received: 70
```

## Conclusion

This *Signal-Slot* system allows elegant and flexible communication between objects in C++. The automatic management of connections during object destruction avoids errors related to manual resource management and simplifies event-driven programming.
