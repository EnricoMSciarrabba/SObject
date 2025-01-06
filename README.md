# SObject - Signal and Slot Framework in C++

## Description
SObject is a C++ library designed to replicate the Signal and Slot mechanism, similar to the one available in frameworks like Qt. This implementation enables the connection of signals to slots, allowing for asynchronous and reactive communication between objects. By using this library, developers can design modular and flexible systems, ideal for GUI applications, event-driven systems, or any scenario requiring a publisher-subscriber communication paradigm.

The library leverages templates to provide the flexibility typical of C++ and includes automatic connection management to ensure resources are deallocated safely.

### Funzionalità principali

1. **Connect**: Permette di connettere un segnale emesso da un oggetto (`emitter`) a una slot di un altro oggetto (`receiver`).
2. **Disconnect**: Permette di rimuovere una connessione tra un segnale e una slot.
3. **Gestione automatica delle connessioni**: Quando un `SObject` viene distrutto, tutte le sue connessioni vengono automaticamente rimosse, evitando memory leaks e mantenendo il sistema pulito.

## Come funziona il sistema di Connect

Il sistema si basa su una connessione tra due componenti principali:

- **Signal**: rappresenta un segnale emesso da un oggetto.
- **Slot**: rappresenta una funzione che reagisce al segnale, ossia una callback associata al segnale.

Quando un oggetto emette un segnale, le slot connesse a quel segnale vengono eseguite.

### La funzione `connect`

La funzione `connect` stabilisce una connessione tra un segnale emesso da un oggetto (`emitter`) e una slot di un altro oggetto (`receiver`). La connessione è stabile finché non viene esplicitamente disconnessa.

#### Sintassi

```cpp
connect(emitter, &EmitterClass::signalMethod, receiver, &ReceiverClass::slotMethod);
```

- `emitter`: Oggetto che emette il segnale.
- `signalMethod`: Metodo membro di `emitter` che rappresenta il segnale.
- `receiver`: Oggetto che riceve il segnale e la cui slot deve essere eseguita.
- `slotMethod`: Metodo membro di `receiver` che rappresenta la slot.

### Come funziona la funzione `connect`:

1. Viene creato un oggetto temporaneo per identificare il segnale e la slot.
2. La connessione viene aggiunta nella mappa dei segnali dell'emettitore.
3. Se non esiste già una connessione simile, viene creato un nuovo record.
4. Viene registrato l'emettitore nella lista del ricevitore, garantendo che il ricevitore possa "ascoltare" i segnali dell'emettitore.

### La funzione `disconnect`

La funzione `disconnect` rimuove una connessione tra un segnale e una slot. Ci sono vari metodi per la disconnessione:

#### Sintassi per disconnettere una specifica slot

```cpp
disconnect(emitter, &EmitterClass::signalMethod, receiver, &ReceiverClass::slotMethod);
```

#### Sintassi per disconnettere tutte le slot di un ricevitore per un segnale specifico

```cpp
disconnect(emitter, &EmitterClass::signalMethod, receiver);
```

#### Sintassi per disconnettere tutte le slot per un segnale

```cpp
disconnect(emitter, &EmitterClass::signalMethod);
```

#### Sintassi per disconnettere tutte le connessioni di un emettitore

```cpp
disconnect(emitter);
```

### Come funziona la funzione `disconnect`:

1. **Disconnessione singola**: La funzione rimuove una connessione specifica tra un segnale e una slot. Dopo aver rimosso la connessione, se non ci sono altre slot per quel segnale, il segnale viene eliminato dalla mappa dell'emettitore.
   
2. **Disconnessione per ricevitore**: Disconnette tutte le slot di un ricevitore per un segnale specifico.

3. **Disconnessione per segnale**: Rimuove tutte le connessioni tra un segnale e le relative slot.

4. **Disconnessione globale**: Rimuove tutte le connessioni di un emettitore (segnali e slot associati).

### Gestione automatica delle connessioni durante la distruzione di un `SObject`

Quando un oggetto di tipo `SObject` viene distrutto, il sistema si occupa di rimuovere automaticamente tutte le connessioni a questo oggetto. In pratica, ogni volta che un oggetto viene distrutto:

1. Vengono rimosse tutte le connessioni da lui emesse.
2. Vengono rimosse tutte le connessioni a lui ricevute.
3. Viene effettuata una pulizia della memoria, rimuovendo tutte le slot che non sono più associate a segnali.

Questo evita possibili memory leaks e mantiene il sistema pulito.

## Strutture e classi principali

### `SObject`

La classe base che rappresenta un oggetto che può emettere segnali e ricevere slot. Può essere connesso a un altro oggetto tramite la funzione `connect` e può disconnettersi tramite la funzione `disconnect`.

- **Funzioni principali**:
  - `emitSignal`: Emette un segnale.
  - `getAllReceivers`: Restituisce tutti i ricevitori associati a un segnale.
  - `signalIsPresent`: Controlla se un segnale è associato a qualche slot.
  - `connectedWithObject`: Controlla se l'oggetto è connesso con un altro.

### `_Signal` e `_Slot`

Queste sono le classi che gestiscono i segnali e le slot. Un segnale rappresenta una funzione che un oggetto emette, mentre una slot rappresenta una funzione di callback a cui un oggetto si può collegare.

### Funzioni di supporto

- **`CustomSignalCompare`**: Utilizzata per confrontare segnali durante la ricerca e la disconnessione.
- **`CustomSlotCompare`**: Utilizzata per confrontare slot e per la disconnessione di slot da segnali o riceventi.

## Esempio

```cpp
class MyObject : public SObject
{
public:
    void signalMethod(int value)
    {
        emitSignal(&MyObject::signalMethod, value);
    }

    void slotMethod(int value)
    {
        std::cout << "Slot received value: " << value << std::endl;
    }
};

int main()
{
    MyObject emitter;
    MyObject receiver;

    connect(&emitter, &MyObject::signalMethod, &receiver, &MyObject::slotMethod);
    
    emitter.signalMethod(42); // Il receiver eseguirà slotMethod(42)
    
    disconnect(&emitter, &MyObject::signalMethod, &receiver, &MyObject::slotMethod);
}
```

## Conclusione

Questo sistema di *Signal-Slot* permette di gestire in modo elegante e flessibile la comunicazione tra oggetti in C++. La gestione automatica delle connessioni durante la distruzione degli oggetti evita errori legati alla gestione manuale delle risorse e semplifica la programmazione orientata agli eventi.
