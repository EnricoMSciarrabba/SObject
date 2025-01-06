#ifndef SOBJECT_H
#define SOBJECT_H

#include <any>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <memory>
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
//            InvokeSlot
//
// =======================================

template <typename T, typename... Args, std::size_t... Is>
void invokeSlotImpl(T* object, void (T::*method)(Args...), std::vector<std::any>& args, std::index_sequence<Is...>)
{
    // Casto ogni std::any al tipo corretto e invocos il metodo
    (object->*method)(std::any_cast<Args>(args[Is])...);
}

template <typename T, typename... Args>
void invokeSlot(T* object, void (T::*method)(Args...), std::vector<std::any>& args)
{
    // Converto e invoco il metodo
    invokeSlotImpl(object, method, args, std::index_sequence_for<Args...>{});
}



// =======================================
//
//              SignalBase
//
// =======================================

class _SignalBase
{
public:
    virtual ~_SignalBase() {};

    // Confronto di due SignalBase tramite puntatori
    virtual bool compareByPointer(const _SignalBase* other) const = 0;

    bool compareByEmitter(const SObject* emitter) const
    {
        return m_emitter == emitter;
    }

protected:
    // Costruttori protected in modo che solo Signal possa creare oggetti di questo tipo
    _SignalBase() = delete;
    _SignalBase(const _SignalBase&) = delete;
    _SignalBase(const SObject* emitter) : m_emitter(emitter){};

    // Emettitore del segnale
    const SObject* m_emitter;
};



// =======================================
//
//               Signal
//
// =======================================

template <typename S>
class _Signal : public _SignalBase
{
public:
    // Anche se i costruttori sono pubblici non utilizzare le seguenti classi
    _Signal() = delete;
    _Signal(const _Signal&) = delete;
    _Signal(const SObject* objectPtr, const S objectMethod) : _SignalBase(objectPtr), m_signal(objectMethod){};
    virtual ~_Signal() {};

    // Confronto di due SignalBase tramite puntatori (override)
    virtual bool compareByPointer(const _SignalBase* other) const override
    {
        const _Signal<S>* signal = dynamic_cast<const _Signal<S>*>(other);
        if(signal == nullptr) return false;

        return m_emitter == signal->m_emitter and
               m_signal  == signal->m_signal;
    }

private:
    // Segnale
    const S m_signal;
};



// =======================================
//
//              SlotBase
//
// =======================================

class _SlotBase
{
public:
    virtual ~_SlotBase() {};

    // Metodi virtuali
    virtual bool compareByPointer(const _SlotBase* other) const = 0;
    virtual bool compareByReceiver(const SObject* receiver) const = 0;
    virtual void exec() = 0;
    virtual SObject* getReceiver() = 0;

    // Metodo per salvare i parametri in input
    void setInputParams(const std::vector<std::any>& inputParams)
    {
        m_inputParams = inputParams;
    }

protected:
    // Costruttori protected in modo che solo Slot possa creare oggetti di questo tipo
    _SlotBase() = default;
    _SlotBase(const _SlotBase&) = delete;

    // Lista dei parametri da dare in input alla slot
    std::vector<std::any> m_inputParams;
};



// =======================================
//
//                Slot
//
// =======================================

template <typename O, typename S>
class _Slot : public _SlotBase
{
public:
    // Anche se i costruttori sono pubblici non utilizzare le seguenti classi
    _Slot() = delete;
    _Slot(const _Slot&) = delete;
    _Slot(O objectPtr, S objectMethod) : m_receiver(objectPtr), m_slot(objectMethod) {};
    virtual ~_Slot() {};

    // Confronto di due SlotBase tramite puntatori (override)
    virtual bool compareByPointer(const _SlotBase* other) const override
    {
        // Provo ad effettuare il cast
        const _Slot<O, S>* slot = dynamic_cast<const _Slot<O, S>*>(other);
        if(slot == nullptr) return false;

        return m_receiver == slot->m_receiver and
               m_slot     == slot->m_slot;
    }

    // Confronto tra ricevitori
    virtual bool compareByReceiver(const SObject* receiver) const override
    {
        return m_receiver == receiver;
    }

    // Metodo per eseguire la slot
    virtual void exec() override
    {
        invokeSlot(m_receiver, m_slot, m_inputParams);
    }

    // Getter receiver
    virtual SObject* getReceiver() override
    {
        return m_receiver;
    }

private:
    O m_receiver;
    S m_slot;
};



// =======================================
//
//          CustomSignalCompare
//
// =======================================

struct CustomSignalCompare
{
    CustomSignalCompare() = delete;
    CustomSignalCompare(const _SignalBase* signal, const bool& deleteObject= false) : m_deleteObject(deleteObject), m_signal(signal){};

    bool operator()(const std::pair<const _sobject::_SignalBase*, std::list<_sobject::_SlotBase*>>& element) const
    {
        // Controllo se il segnale chiave è uguale a quello che si sta cercando
        return element.first->compareByPointer(m_signal);
    }

    bool operator()(const _sobject::_SignalBase* element) const
    {
        // Controllo se il segnale chiave è uguale a quello che si sta cercando
        bool result = element->compareByPointer(m_signal);

        // Controllo se eliminare il segnale
        if(result and m_deleteObject)
        {
            delete element;
        }

        return true;
    }

    const bool m_deleteObject   = false;
    const _SignalBase* m_signal = nullptr;
};



// =======================================
//
//          CustomSlotCompare
//
// =======================================

struct CustomSlotCompare
{
    CustomSlotCompare() = delete;
    CustomSlotCompare(const _SlotBase* slot,   const bool& deleteObject= true) : m_deleteObject(deleteObject), m_slot(slot){};
    CustomSlotCompare(const SObject* receiver, const bool& deleteObject= true) : m_deleteObject(deleteObject), m_receiver(receiver){};

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

    const bool m_deleteObject = true;
    const _SlotBase* m_slot   = nullptr;
    const SObject* m_receiver = nullptr;
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
        std::list<_sobject::_SignalBase const*> listToremove;
        for(SObject* sObject : m_slotToSignalObjectList)
        {
            // Svuoto la lista di liste da riuovere
            listToremove.clear();

            // Per ogni segnale di tale oggetto
            for(auto& signal : sObject->m_signalToSlotMap)
            {
                // Elimino le slot dell'oggetto corrente
                signal.second.remove_if(_sobject::CustomSlotCompare(this));

                // Se la lista è vuota aggiungo il riferimento per eliminarla successivamente
                if(signal.second.empty())
                {
                    listToremove.push_back(signal.first);
                }
            }

            // Una volta finita la rimozione rimuovo le liste vuote
            for(auto keyToRemove : listToremove)
            {
                delete keyToRemove;

                sObject->m_signalToSlotMap.erase(keyToRemove);
            }
        }

        m_slotToSignalObjectList.clear();
    };



    // ===============================
    //
    //  Emit

protected:
    template <typename SM, typename... Args>
    void emitSignal(SM signalMethod, Args&&... args) const
    {
        // Creo un segnale temporaneo per cercarlo
        const _sobject::_SignalBase* signal = new _sobject::_Signal<SM>(this, signalMethod);

        // Cerco il segnale
        auto it = std::find_if(m_signalToSlotMap.begin(), m_signalToSlotMap.end(), _sobject::CustomSignalCompare(signal));
        if(it == m_signalToSlotMap.end())
        {
            delete signal;
            return;
        }

        // Per ogni slot associata al segnale
        for(_sobject::_SlotBase* connectedObject : it->second)
        {
            // Salvo i parametri
            connectedObject->setInputParams({std::forward<Args>(args)...});

            // Eseguo la slot
            connectedObject->exec();
        }

        delete signal;
    }



    // ===============================
    //
    //  Metodi esterni

public:
    bool signalIsPresent(_sobject::_SignalBase* signal) const
    {
        return m_signalToSlotMap.end() !=
               std::find_if(m_signalToSlotMap.begin(),
                            m_signalToSlotMap.end(),
                            _sobject::CustomSignalCompare(signal));
    }

    bool connectedWithObject(SObject* receiver) const
    {
        // Per ogni segnale
        for(auto& pairMap : m_signalToSlotMap)
        {
            // Controllo se ci sono slot del receiver
            if(pairMap.second.end() !=
               std::find_if(pairMap.second.begin(),
                            pairMap.second.end(),
                            _sobject::CustomSlotCompare(receiver, false)))
            {
                return true;
            }
        }

        return false;
    }

    std::list<SObject*> getAllReceivers(_sobject::_SignalBase* signal = nullptr) const
    {
        // Creo la lista dei ricevitori
        std::list<SObject*> allReceiver;

        // Per ogni segnale
        for(auto& pairMap : m_signalToSlotMap)
        {
            // Se si vogliono i riceiver di un solo signal controllo se è quello in input
            if(signal != nullptr and not pairMap.first->compareByPointer(signal)) continue;

            // Per ogni slot associata al segnale
            for(auto slot : pairMap.second)
            {
                // Salvo il receiver
                allReceiver.push_back(slot->getReceiver());
            }
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
    void removeSignalSlotConnection(_sobject::_SignalBase* signal, _sobject::_SlotBase* slot)
    {
        // Recupero l'iteratore nella mappa del segnale
        auto it = std::find_if(m_signalToSlotMap.begin(), m_signalToSlotMap.end(), _sobject::CustomSignalCompare(signal));

        // Controllo se il segnale non è presente
        if(it == m_signalToSlotMap.end()) return;

        // Rimuovo le slot in input associate al segnale
        it->second.remove_if(_sobject::CustomSlotCompare(slot));

        // Se al segnale non è associata nessuna slot
        if(it->second.empty())
        {
            // Dealloco la memoria della chiave
            delete it->first;

            // Elimino il riferimento del segnale nella mappa
            m_signalToSlotMap.erase(it);
        }
    }

    void removeSignalReceiverSlotConnection(_sobject::_SignalBase* signal, SObject* receiver)
    {
        // Recupero l'iteratore nella mappa del segnale
        auto it = std::find_if(m_signalToSlotMap.begin(), m_signalToSlotMap.end(), _sobject::CustomSignalCompare(signal));

        // Controllo se il segnale non è presente
        if(it == m_signalToSlotMap.end()) return;

        // Rimuovo le slot associate al receiver
        it->second.remove_if(_sobject::CustomSlotCompare(receiver));

        // Se al segnale non è associata nessuna slot
        if(it->second.empty())
        {
            // Dealloco la memoria della chiave
            delete it->first;

            // Elimino il riferimento del segnale nella mappa
            m_signalToSlotMap.erase(it);
        }
    }

    void removeAllSignalSlotConnection(_sobject::_SignalBase* signal)
    {
        // Recupero l'iteratore nella mappa del segnale
        auto it = std::find_if(m_signalToSlotMap.begin(), m_signalToSlotMap.end(), _sobject::CustomSignalCompare(signal));

        // Controllo se il segnale non è presente
        if(it == m_signalToSlotMap.end()) return;

        // Rimuovo tutte le slot
        for(auto slot : it->second) delete slot;
        it->second.clear();

        // Poichè al segnale non è associata nessuna slot
        // Dealloco la memoria della chiave ed elimino il riferimento del segnale nella mappa
        delete it->first;
        m_signalToSlotMap.erase(it);
    }

    void removeAllConnection()
    {
        // Per ogni segnale
        for(auto& pair : m_signalToSlotMap)
        {
            // Dealloco tutti i valori delle slot
            for(auto& slot : pair.second) delete slot;

            // Clear della lista
            pair.second.clear();

            // Dealloco chiave
            delete pair.first;
        }

        // Clear della mappa
        m_signalToSlotMap.clear();
    }



    // ===============================
    //
    //  Strutture interne

private:
    std::map<const _sobject::_SignalBase*, std::list<_sobject::_SlotBase*>> m_signalToSlotMap;
    std::list<SObject*> m_slotToSignalObjectList;



    // ===============================
    //
    //  Friend

    template<typename E, typename R, typename... Args>
    friend void connect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...));

    template<typename E, typename R, typename... Args>
    friend void disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...));

    template<typename E, typename R, typename... Args>
    friend void disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver);

    template<typename E, typename... Args>
    friend void disconnect(SObject* emitter, void(E::*signalM)(Args...));

    friend void disconnect(SObject* emitter);
};









// =======================================
//
//               Connect
//
// =======================================

template<typename E, typename R, typename... Args>
void connect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...))
{
    // Creo l'oggetto per identificare il segnale
    const _sobject::_SignalBase* signal = new _sobject::_Signal<void(E::*)(Args...)>(emitter, signalM);
    bool deleteSignal = false;

    // Creo l'oggetto per identificare la slot
    _sobject::_SlotBase* slot = new _sobject::_Slot<R*, void(R::*)(Args...)>(receiver, slotM);

    // Controllo se l'emettitore ha già una lista associata al segnale
    auto mapIt = std::find_if(emitter->m_signalToSlotMap.begin(), emitter->m_signalToSlotMap.end(), _sobject::CustomSignalCompare(signal));
    if(mapIt != emitter->m_signalToSlotMap.end())
    {
        // Inserisco il segnale nella lista
        mapIt->second.push_back(slot);

        // Il nuovo oggetto che identifica il segnale deve essere eliminato
        deleteSignal = true;
    }
    else
    {
        // Se è la prima connect creo la lista
        emitter->m_signalToSlotMap[signal] = std::list<_sobject::_SlotBase*>{slot};
    }

    // Controllo se il ricevitore non ha il segnale registrato
    auto listIt = std::find(receiver->m_slotToSignalObjectList.begin(), receiver->m_slotToSignalObjectList.end(), emitter);
    if(listIt == receiver->m_slotToSignalObjectList.end())
    {
        // Registro il segnale
        receiver->m_slotToSignalObjectList.push_back(emitter);
        deleteSignal = false;
    }

    // Controllo se eliminare signal e slot temporanei
    if(deleteSignal)
    {
        delete signal;
    }
}



// =======================================
//
//             Disconnect
//
// =======================================

template<typename E, typename R, typename... Args>
void disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver, void(R::*slotM)(Args...))
{
    // Creo l'oggetto per identificare il segnale e la slot
    std::unique_ptr<_sobject::_SignalBase> signal = std::make_unique<_sobject::_Signal<void(E::*)(Args...)>>(emitter, signalM);
    std::unique_ptr<_sobject::_SlotBase>   slot   = std::make_unique<_sobject::_Slot<R*, void(R::*)(Args...)>>(receiver, slotM);

    // Controllo se esistono connect con il seguente segnale
    if(not emitter->signalIsPresent(signal.get())) return;

    // Prima lavoro sull'emitter
    // Rimuovo i collegamenti nell'emitter tra signal e slot
    emitter->removeSignalSlotConnection(signal.get(), slot.get());

    // Dopo lavoro sul receiver
    // Controllo che tra tutti i signal dell'emitter non ci siano connect con slot del receiver
    if(not emitter->connectedWithObject(receiver))
    {
        // Rimuovo l'emitter dal receiver
        dynamic_cast<SObject*>(receiver)->m_slotToSignalObjectList.remove(emitter);
    }
}

template<typename E, typename R, typename... Args>
void disconnect(SObject* emitter, void(E::*signalM)(Args...), R* receiver)
{
    // Creo l'oggetto per identificare il segnale
    std::unique_ptr<_sobject::_SignalBase> signal = std::make_unique<_sobject::_Signal<void(E::*)(Args...)>>(emitter, signalM);

    // Controllo se esistono connect con il seguente segnale
    if(not emitter->signalIsPresent(signal.get())) return;

    // Prima lavoro sull'emitter
    // Rimuovo i collegamenti nell'emitter tra signal e tutte le slot del receiver
    emitter->removeSignalReceiverSlotConnection(signal.get(), receiver);

    // Dopo lavoro sul receiver
    // Controllo che tra tutti i signal dell'emitter non ci siano connect con slot del receiver
    if(not emitter->connectedWithObject(receiver))
    {
        // Rimuovo l'emitter dal receiver
        dynamic_cast<SObject*>(receiver)->m_slotToSignalObjectList.remove(emitter);
    }
}

template<typename E, typename... Args>
void disconnect(SObject* emitter, void(E::*signalM)(Args...))
{
    // Creo l'oggetto per identificare il segnale
    std::unique_ptr<_sobject::_SignalBase> signal = std::make_unique<_sobject::_Signal<void(E::*)(Args...)>>(emitter, signalM);

    // Controllo se esistono connect con il seguente segnale
    if(not emitter->signalIsPresent(signal.get())) return;

    // Recupero tutti i receiver associati al segnale
    std::list<SObject*> receiverList = emitter->getAllReceivers(signal.get());

    // Rimuovo tutti i collegamenti nell'emitter tra signal e tutte le slot
    emitter->removeAllSignalSlotConnection(signal.get());

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
