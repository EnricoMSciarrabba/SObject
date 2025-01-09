#ifndef SOBJECT_H
#define SOBJECT_H

#include <list>
#include <algorithm>
#include <cstdint>

#define S_SIGNAL
#define S_SLOT

class SObject;

/* ===========================================================================
 *
 *    Nel seguente namespace (_sobject) vengono inserite tutte le classi e
 *    le strutture alle quali l'utente non deve avere accesso.
 *    Per le classi e i metodi da utilizzare saltare il namespace.
 *
 * ===========================================================================
 */

namespace _sobject
{

// =======================================
//
//              SlotBase
//
// =======================================

template <typename... Args>
class _SlotBase
{
protected:
    // Costruttori protected in modo che solo Slot possa creare oggetti di questo tipo
    _SlotBase() = default;
    _SlotBase(const _SlotBase&) = delete;

public:
    virtual ~_SlotBase() {};



    // ===============================
    //
    //  Metodi astratti

public:
    virtual bool compareByPointer(const _SlotBase* other) const = 0;
    virtual bool compareByReceiver(const SObject* receiver) const = 0;
    virtual SObject* getReceiver() const = 0;
    virtual void exec(Args&&...) = 0;



    // ===============================
    //
    //  CustomSlotCompare

public:
    struct CustomSlotCompare
    {
        CustomSlotCompare() = delete;
        CustomSlotCompare(const _SlotBase<Args...>* slot, const bool& deleteObject= true) : m_deleteObject(deleteObject), m_slot(slot){};
        CustomSlotCompare(const SObject* receiver,        const bool& deleteObject= true) : m_deleteObject(deleteObject), m_receiver(receiver){};

        bool operator()(const _SlotBase* slot) const
        {
            bool result = false;

            // Controllo se rimuovo per slot o per ricevitore
            if(m_slot     != nullptr) result = slot->compareByPointer(m_slot);
            if(m_receiver != nullptr) result = slot->compareByReceiver(m_receiver);

            // Se devo rimuovere la slot dealloco la memoria
            if(result and m_deleteObject)
            {
                delete slot;
            }

            return result;
        }

        const bool m_deleteObject        = true;
        const _SlotBase<Args...>* m_slot = nullptr;
        const SObject* m_receiver        = nullptr;
    };
};



// =======================================
//
//                Slot
//
// =======================================

template <typename Receiver, typename... Args>
class _Slot : public _SlotBase<Args...>
{
public:
    // Anche se i costruttori sono pubblici non utilizzare le seguenti classi
    _Slot() = delete;
    _Slot(const _Slot&) = delete;
    _Slot(Receiver* receiver, void(Receiver::*method)(Args...)) : m_receiver(receiver), m_method(method) {};
    virtual ~_Slot() {};



    // ===============================
    //
    //  Override

    // Confronto di due tramite puntatori (override)
    virtual bool compareByPointer(const _SlotBase<Args...>* other) const override
    {
        // Provo ad effettuare il cast
        auto slot = dynamic_cast<const _Slot<Receiver, Args...>*>(other);
        if(slot == nullptr) return false;

        return m_receiver == slot->m_receiver and
               m_method   == slot->m_method;
    }

    // Confronto tra ricevitori
    virtual bool compareByReceiver(const SObject* receiver) const override
    {
        return m_receiver == receiver;
    }

    // Getter receiver
    virtual SObject* getReceiver() const override
    {
        return m_receiver;
    }

    // Metodo per eseguire la slot
    virtual void exec(Args&&... args) override
    {
        (m_receiver->*m_method)(std::forward<Args...>(args)...);
    }



    // ===============================
    //
    //  Variabili

private:
    Receiver* m_receiver;
    void(Receiver::*m_method)(Args...);
};



// =======================================
//
//              SignalBase
//
// =======================================

class _SignalBase
{
protected:
    // Costruttori protected in modo che solo Signal possa creare oggetti di questo tipo
    _SignalBase() = default;
    _SignalBase(const _SignalBase&) = delete;

public:
    virtual ~_SignalBase() {};



    // ===============================
    //
    //  Metodi astratti

public:
    virtual bool compareByPointer(const _SignalBase* other) const = 0;
    virtual void removeSlotByReceiver(const SObject* receiver) = 0;
    virtual bool connectedWithObject(const SObject* receiver) = 0;
    virtual std::list<SObject*> getAllReceivers() const = 0;



    // ===============================
    //
    //  CustomSignalCompare

public:
    struct CustomSignalCompare
    {
        CustomSignalCompare() = delete;
        CustomSignalCompare(const _SignalBase* signal, const bool& deleteObject= false) : m_deleteObject(deleteObject), m_signal(signal){};

        bool operator()(const _sobject::_SignalBase* signal) const
        {
            // Controllo se il segnale chiave è uguale a quello che si sta cercando
            bool result = signal->compareByPointer(m_signal);

            // Controllo se eliminare il segnale
            if(result and m_deleteObject)
            {
                delete signal;
            }

            // Restituisco il risultato
            return result;
        }

        const bool m_deleteObject   = false;
        const _SignalBase* m_signal = nullptr;
    };
};



// =======================================
//
//               Signal
//
// =======================================

template <typename Emitter, typename... Args>
class _Signal : public _SignalBase
{
public:
    // Anche se i costruttori sono pubblici non utilizzare le seguenti classi
    _Signal() = delete;
    _Signal(const _Signal&) = delete;
    _Signal(void(Emitter::* const signal)(Args...)) : m_signal(signal){};
    virtual ~_Signal()
    {
        // Per ogni slot
        for(auto slot : m_slots)
        {
            // Dealloco oggetto
            delete slot;
        }
    };



    // ===============================
    //
    //  Override

public:
    // Confronto di due SignalBase tramite puntatori (override)
    virtual bool compareByPointer(const _SignalBase* other) const override
    {
        auto signal = dynamic_cast<const _Signal<Emitter, Args...>*>(other);
        if(signal == nullptr) return false;

        return m_signal  == signal->m_signal;
    }

    // Rimuovo tutte le slot del receiver
    virtual void removeSlotByReceiver(const SObject* receiver) override
    {
        m_slots.remove_if(typename _SlotBase<Args...>::CustomSlotCompare(receiver));
    }

    virtual bool connectedWithObject(const SObject* receiver) override
    {
        // Controllo se ci sono slot del receiver
        return m_slots.end() !=
                    std::find_if(m_slots.begin(),
                    m_slots.end(),
                    typename _sobject::_SlotBase<Args...>::CustomSlotCompare(receiver, false));
    }

    virtual std::list<SObject*> getAllReceivers() const override
    {
        std::list<SObject*> list_t;

        for(_SlotBase<Args...>* slot : m_slots)
        {
            list_t.push_back(slot->getReceiver());
        }

        return list_t;
    }



    // ===============================
    //
    //  Interfacce esterne

    template <typename Receiver>
    void addSlot(_sobject::_Slot<Receiver, Args...>* slot)
    {
        m_slots.push_back(slot);
    }

    void removeSlot(SObject* receiver)
    {
        m_slots.remove_if(typename _SlotBase<Args...>::CustomSlotCompare(receiver));
    }

    template <typename Receiver>
    void removeSlot(_sobject::_Slot<Receiver, Args...>* slot)
    {
        m_slots.remove_if(typename _SlotBase<Args...>::CustomSlotCompare(slot));
    }

    void execAllSlots(Args&&... args)
    {
        for(_SlotBase<Args...>* slot : m_slots)
        {
            slot->exec(std::forward<Args...>(args)...);
        }
    }



    // ===============================
    //
    //  Variabili

private:
    // Segnale
    void(Emitter::* const m_signal)(Args...);
    std::list<_SlotBase<Args...>*> m_slots;
};

} // namespace _sobject









/* ===========================================================================
 *
 *    Di seguito sono presenti le classi e i metodi da utilizzare.
 *
 * ===========================================================================
 */

// =======================================
//
//               SObject
//
// =======================================

class SObject
{
public:
    SObject(){};
    virtual ~SObject()
    {
        // Effettuo il reset delle connect
        disconnect(this);

        // Per ogni SObject che ha una connect con il seguente oggetto
        for(SObject* sObject : m_slotToSignalObjectList)
        {
            // Per ogni segnale di tale oggetto
            for(auto signal : sObject->m_signalsList)
            {
                signal->removeSlotByReceiver(this);
            }
        }

        m_slotToSignalObjectList.clear();
    };



    // ===============================
    //
    //  Emit

protected:
    template <typename Emitter, typename... Args>
    void emitSignal(void(Emitter::* const signalM)(Args...), Args... args) const
    {
        // Creo un segnale temporaneo per cercarlo
        auto signal = new _sobject::_Signal<Emitter, Args...>(signalM);

        // Cerco il segnale
        auto signalIterator = std::find_if(m_signalsList.begin(), m_signalsList.end(), _sobject::_SignalBase::CustomSignalCompare(signal));
        if(signalIterator == m_signalsList.end())
        {
            delete signal;
            return;
        }

        // Effettuo il cast al tipo di segnale
        _sobject::_Signal<Emitter, Args...>* slotContainer = dynamic_cast<_sobject::_Signal<Emitter, Args...>*>(*signalIterator);
        if(slotContainer == nullptr)
        {
            delete signal;
            return;
        }

        // Chiamo tutte le slot
        slotContainer->execAllSlots(std::forward<Args...>(args)...);

        delete signal;
        return;
    }



    // ===============================
    //
    //  Metodi esterni

public:
    bool connectedWithObject(SObject* receiver) const
    {
        // Per ogni segnale
        for(auto signal : m_signalsList)
        {
            if(signal->connectedWithObject(receiver))
            {
                return true;
            }
        }

        return false;
    }

    std::list<SObject*> getAllReceivers(_sobject::_SignalBase* signalIn = nullptr) const
    {
        // Creo la lista dei ricevitori
        std::list<SObject*> allReceiver;

        // Per ogni segnale
        for(const _sobject::_SignalBase* signal : m_signalsList)
        {
            // Se si vogliono i receiver di un solo signal controllo se è quello in input
            if(signal == nullptr or not signal->compareByPointer(signalIn)) continue;

            // Recupero tutti i receiver del segnale
            std::list<SObject*> signalReceivers = signal->getAllReceivers();

            // Salvo tutti i receiver di questo segnale
            allReceiver.insert(allReceiver.begin(), signalReceivers.begin(), signalReceivers.end());
        }

        // Ordino tutti i receiver (per poi chiamare unique)
        allReceiver.sort([](SObject* obj1, SObject* obj2)
        {
            return reinterpret_cast<uintptr_t>(obj1) < reinterpret_cast<uintptr_t>(obj2);
        });

        // Chiamo unique ed elimino le ripetizioni
        auto _allReceiver = std::unique(allReceiver.begin(), allReceiver.end(), [](SObject* obj1, SObject* obj2)
        {
            return reinterpret_cast<uintptr_t>(obj1) == reinterpret_cast<uintptr_t>(obj2);
        });
        allReceiver.erase(_allReceiver, allReceiver.end());

        return allReceiver;
    }



    // ===============================
    //
    //  Metodi interni

private:
    void removeAllConnection()
    {
        // Per ogni segnale
        for(auto signal : m_signalsList)
        {
            // Elimino segnale
            delete signal;
        }

        // Clear della mappa
        m_signalsList.clear();
    }



    // ===============================
    //
    //  Strutture interne

private:
    std::list<_sobject::_SignalBase*> m_signalsList;
    std::list<SObject*> m_slotToSignalObjectList;



    // ===============================
    //
    //  Friend

    template<typename E, typename R, typename... Args>
    friend void connect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...));

    template<typename Emitter, typename Receiver, typename... Args>
    friend void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver, void(Receiver::*slotM)(Args...));

    template<typename Emitter, typename Receiver, typename... Args>
    friend void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver);

    template<typename Emitter, typename... Args>
    friend void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...));

    friend void disconnect(SObject* emitter);
};









// =======================================
//
//               Connect
//
// =======================================

template<typename Emitter, typename Receiver, typename... Args>
void connect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver, void(Receiver::*slotM)(Args...))
{
    // Creo l'oggetto per identificare il segnale
    _sobject::_Signal<Emitter, Args...>* signal = new _sobject::_Signal<Emitter, Args...>(signalM);

    // Creo l'oggetto per identificare la slot
    _sobject::_Slot<Receiver, Args...>* slot = new _sobject::_Slot<Receiver, Args...>(receiver, slotM);

    // Controllo se il segnale ha delle slot registrate
    auto signalFound = std::find_if(emitter->m_signalsList.begin(), emitter->m_signalsList.end(), _sobject::_SignalBase::CustomSignalCompare(signal));
    if(signalFound != emitter->m_signalsList.end())
    {
        // Effettuo il cast al tipo di segnale
        _sobject::_Signal<Emitter, Args...>* slotContainer = dynamic_cast<_sobject::_Signal<Emitter, Args...>*>(*signalFound);
        if(slotContainer == nullptr)
        {
            delete signal;
            delete slot;
            return;
        }

        // Salvo la nuova slot
        slotContainer->addSlot(slot);

        delete signal;
    }
    else
    {
        // Se è la prima connect aggiungo la slot alla connect e la signal all'emitter
        signal->addSlot(slot);
        emitter->m_signalsList.push_back(signal);
    }

    // Controllo se il ricevitore non ha il segnale registrato
    auto listIt = std::find(receiver->m_slotToSignalObjectList.begin(), receiver->m_slotToSignalObjectList.end(), emitter);
    if(listIt == receiver->m_slotToSignalObjectList.end())
    {
        // Registro il segnale
        receiver->m_slotToSignalObjectList.push_back(emitter);
    }
}



// =======================================
//
//             Disconnect
//
// =======================================

template<typename Emitter, typename Receiver, typename... Args>
void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver, void(Receiver::*slotM)(Args...))
{
    // Creo l'oggetto per identificare il segnale e la slot
    _sobject::_Signal<Emitter, Args...> *signal = new _sobject::_Signal<Emitter, Args...>(signalM);
    _sobject::_Slot<Receiver, Args...>  *slot   = new _sobject::_Slot<Receiver, Args...>(receiver, slotM);

    // Prima lavoro sull'emitter
    // Trovo il segnale nell'emitter
    auto emitterSignal = std::find_if(emitter->m_signalsList.begin(), emitter->m_signalsList.end(), _sobject::_SignalBase::CustomSignalCompare(signal));
    if(emitterSignal == emitter->m_signalsList.end())
    {
        delete signal;
        delete slot;
        return;
    }

    // Effettuo il cast
    _sobject::_Signal<Emitter, Args...>* signalT = dynamic_cast<_sobject::_Signal<Emitter, Args...>*>(*emitterSignal);
    if(signalT == nullptr)
    {
        delete signal;
        delete slot;
        return;
    }

    // Rimuovo slot
    signalT->removeSlot(slot);

    // Dopo lavoro sul receiver
    // Controllo che tra tutti i signal dell'emitter non ci siano connect con slot del receiver
    if(not emitter->connectedWithObject(receiver))
    {
        // Rimuovo l'emitter dal receiver
        dynamic_cast<SObject*>(receiver)->m_slotToSignalObjectList.remove(emitter);
    }

    delete signal;
    delete slot;
    return;
}

template<typename Emitter, typename Receiver, typename... Args>
void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...), Receiver* receiver)
{
    // Creo l'oggetto per identificare il segnale
    _sobject::_SignalBase* signal = new _sobject::_Signal<Emitter, Args...>(signalM);

    // Prima lavoro sull'emitter
    // Cerco il segnale
    auto emitterSignal = std::find_if(emitter->m_signalsList.begin(), emitter->m_signalsList.end(), _sobject::_SignalBase::CustomSignalCompare(signal));
    if(emitterSignal == emitter->m_signalsList.end())
    {
        delete signal;
        return;
    }

    // Effettuo il cast
    _sobject::_Signal<Emitter, Args...>* signalT = dynamic_cast<_sobject::_Signal<Emitter, Args...>*>(*emitterSignal);
    if(signalT == nullptr)
    {
        delete signal;
        return;
    }

    // Rimuovo slot
    signalT->removeSlot(receiver);

    // Dopo lavoro sul receiver
    // Controllo che tra tutti i signal dell'emitter non ci siano connect con slot del receiver
    if(not emitter->connectedWithObject(receiver))
    {
        // Rimuovo l'emitter dal receiver
        dynamic_cast<SObject*>(receiver)->m_slotToSignalObjectList.remove(emitter);
    }

    delete signal;
}

template<typename Emitter, typename... Args>
void disconnect(SObject* emitter, void(Emitter::*signalM)(Args...))
{
    // Creo l'oggetto per identificare il segnale
    _sobject::_SignalBase* signal = new _sobject::_Signal<Emitter, Args...>(signalM);

    // Recupero tutti i receiver associati al segnale
    std::list<SObject*> receiverList = emitter->getAllReceivers(signal);

    // Rimuovo il segnale
    emitter->m_signalsList.remove_if(_sobject::_SignalBase::CustomSignalCompare(signal, true));

    // Controllo per tutti i receiver trovati prima se questi hanno altre connect con l'emitter
    for(SObject* receiver : receiverList)
    {
        // Se l'emitter non ha più connect con il receiver
        if(not emitter->connectedWithObject(receiver))
        {
            // Rimuovo l'emitter dal receiver
            receiver->m_slotToSignalObjectList.remove(emitter);
        }
    }

    delete signal;
}

void disconnect(SObject* emitter)
{
    // Recupero tutti i receiver associati ai segnali dell'emitter
    std::list<SObject*> receiverList = emitter->getAllReceivers();

    // Rimuovo tutte le connect dall'emitter
    emitter->removeAllConnection();

    // Per tutti i receiver trovati prima
    for(SObject* receiver : receiverList)
    {
        // Rimuovo l'emitter dal receiver
        receiver->m_slotToSignalObjectList.remove(emitter);
    }
}

#endif // SOBJECT_H
