# SObject - Signal and Slot Framework in C++

## Description
SObject is a C++ library designed to replicate the Signal and Slot mechanism, similar to the one available in frameworks like Qt. This implementation enables the connection of signals to slots, allowing for asynchronous and reactive communication between objects. By using this library, developers can design modular and flexible systems, ideal for GUI applications, event-driven systems, or any scenario requiring a publisher-subscriber communication paradigm.

The library leverages templates to provide the flexibility typical of C++ and includes automatic connection management to ensure resources are deallocated safely.

---

### **Emitting Signals**

- **`emitSignal(SM signalMethod, Args&&... args)`**
  - This method emits a signal to all connected slots.
  - **Parameters:**
    - `signalMethod`: The method of the signal to emit.
    - `args`: Arguments to be passed to the connected slots.

---

### **Connecting Signals and Slots**

You can connect signals and slots using the `connect` function. The framework allows you to connect signals emitted by one object to methods (slots) of another object.

#### **`connect` Syntax:**

- **`connect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...))`**
  - Connect a signal from the emitter to a slot in the receiver.
  - **Parameters:**
    - `emitter`: The object emitting the signal.
    - `signalM`: The signal method in the emitter.
    - `receiver`: The object receiving the signal.
    - `slotM`: The method in the receiver to handle the signal.

---

### **Disconnecting Signals and Slots**

You can disconnect a signal-slot connection with the `disconnect` function.

#### **`disconnect` Syntax:**

- **`disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...))`**
  - Disconnect a specific slot from a specific signal.
  - **Parameters:**
    - `emitter`: The object emitting the signal.
    - `signalM`: The signal method in the emitter.
    - `receiver`: The object receiving the signal.
    - `slotM`: The method in the receiver to handle the signal.

- **`disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver)`**
  - Disconnect all slots from a specific receiver for a given signal.
  - **Parameters:**
    - `emitter`: The object emitting the signal.
    - `signalM`: The signal method in the emitter.
    - `receiver`: The object receiving the signal.

- **`disconnect(SObject* emitter, void(E::*signalM)(Args...))`**
  - Disconnect all slots from all receivers for a given signal.
  - **Parameters:**
    - `emitter`: The object emitting the signal.
    - `signalM`: The signal method in the emitter.

- **`disconnect(SObject* emitter)`**
  - Disconnect all signals and slots associated with the emitter.
  - **Parameters:**
    - `emitter`: The object emitting the signal.

---

### **Getting All Receivers for a Signal**

- **`getAllReceivers(_sobject::_SignalBase* signal = nullptr)`**
  - Retrieve all receivers connected to a given signal.
  - **Parameters:**
    - `signal` (optional): The specific signal to query receivers for.
  - **Returns:**
    - A list of objects (receivers) connected to the signal.

---

### **Additional Methods**

#### **Signal and Slot Existence**

- **`signalIsPresent(_sobject::_SignalBase* signal) const`**
  - Check if a signal is already present and connected.
  - **Parameters:**
    - `signal`: The signal to check.
  - **Returns:**
    - `true` if the signal is connected, otherwise `false`.

- **`connectedWithObject(SObject* receiver) const`**
  - Check if the emitter is connected to a specific receiver.
  - **Parameters:**
    - `receiver`: The receiver to check.
  - **Returns:**
    - `true` if the emitter is connected to the receiver, otherwise `false`.

---

### **Memory Management**

When a **SObject** is destroyed, all connections and memory associated with signals and slots are properly cleaned up to avoid memory leaks.

---
