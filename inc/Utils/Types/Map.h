//
// Created by Monika on 23.07.2026.
//

#if !defined(SR_COMMON_MAP_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_COMMON_MAP_H

#include <Utils/Types/Pair.h>

namespace SR_UTILS_NS {
    // =========================================================================
    // Map — ordered associative container backed by a Red-Black tree.
    //
    // Optimisations vs naive implementation:
    //   • Sentinel nil-node allocated on heap (one per Map instance).
    //     Eliminates nullptr checks in every rotation / fix-up step.
    //   • nil->pParent always tracks m_pRoot  →  O(1) root access.
    //   • m_pMinimum cache  →  O(1) begin().
    //   • Parent pointer in every node  →  O(log n) in-order iterator ++ / --.
    //   • SR_FORCE_INLINE on hot rotation / successor paths.
    //   • operator[] has its own inlined insert path (avoids second tree walk).
    //   • DestroySubtree is iterative on the left spine (minimises stack depth).
    //
    // Complexity: O(log n) insert / erase / find / lower_bound / upper_bound.
    // =========================================================================
    template<
        typename Key,
        typename Value,
        typename Compare = std::less<Key>
    >
    class Map {
    public:
        using SizeType   = size_t;
        using KeyType    = Key;
        using MappedType = Value;
        using ValueType  = Pair<const Key, Value>;
        using value_type = Pair<const Key, Value>;
        using key_type  = Key;
        using mapped_type = Value;
        static constexpr bool key_only = false;

    private:
        enum class Color : uint8_t { Red, Black };

        struct Node {
            ValueType kv;
            Node*  pLeft   = nullptr;
            Node*  pRight  = nullptr;
            Node*  pParent = nullptr;
            Color  color   = Color::Red;

            template<typename K, typename V>
            Node(K&& k, V&& v)
                : kv(std::forward<K>(k), std::forward<V>(v))
            { }

            template<typename K>
            Node(K&& key)
                : kv(
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<K>(key)),
                std::forward_as_tuple()
            )
            { }

            template<typename... Args>
            explicit Node(Args&&... args)
                : kv(std::forward<Args>(args)...)
            { }

            SR_NODISCARD SR_FORCE_INLINE const Key& key() const noexcept { return kv.first; }
        };

    public:
        // ---- Iterator -------------------------------------------------------
        // Stores a Node* and a pointer to the nil sentinel of its owning Map.
        // end() == Iterator pointing at nil.
        struct Iterator {
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = ValueType;
            using reference         = ValueType&;
            using pointer           = ValueType*;

            Iterator prev() const noexcept { return Iterator(Prev(m_p, m_nil), m_nil); }
            Iterator next() const noexcept { return Iterator(Next(m_p, m_nil), m_nil); }

            Iterator() noexcept = default;
            Iterator(const Iterator&) noexcept = default;
            Iterator(Iterator&&) noexcept = default;
            Iterator& operator=(const Iterator&) noexcept = default;
            Iterator& operator=(Iterator&&) noexcept = default;
            Iterator(Node* p, Node* nil) noexcept : m_p(p), m_nil(nil) { }

            SR_NODISCARD ValueType& operator*()  const noexcept { return m_p->kv; }
            SR_NODISCARD ValueType* operator->() const noexcept { return &m_p->kv; }

            SR_FORCE_INLINE Iterator& operator++() noexcept { m_p = Next(m_p, m_nil); return *this; }
            Iterator operator++(int) noexcept { Iterator t = *this; ++*this; return t; }
            SR_FORCE_INLINE Iterator& operator--() noexcept { m_p = Prev(m_p, m_nil); return *this; }
            Iterator operator--(int) noexcept { Iterator t = *this; --*this; return t; }

            SR_NODISCARD bool operator==(const Iterator& o) const noexcept { return m_p == o.m_p; }
            SR_NODISCARD bool operator!=(const Iterator& o) const noexcept { return m_p != o.m_p; }
            SR_NODISCARD Node* GetNode() const noexcept { return m_p; }

        private:
            Node* m_p   = nullptr;
            Node* m_nil = nullptr;
        };

        struct ConstIterator {
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = ValueType;
            using reference         = const ValueType&;
            using pointer           = const ValueType*;

            ConstIterator() noexcept = default;
            ConstIterator(const Node* p, const Node* nil) noexcept : m_p(p), m_nil(nil) { }
            ConstIterator(Iterator it) noexcept : m_p(it.GetNode()), m_nil(nullptr) { } /// NOLINT

            ConstIterator prev() const noexcept { return ConstIterator(Prev(m_p, m_nil), m_nil); }
            ConstIterator next() const noexcept { return ConstIterator(Next(m_p, m_nil), m_nil); }

            SR_NODISCARD const ValueType& operator*()  const noexcept { return m_p->kv; }
            SR_NODISCARD const ValueType* operator->() const noexcept { return &m_p->kv; }

            SR_FORCE_INLINE ConstIterator& operator++() noexcept { m_p = Next(m_p, m_nil); return *this; }
            ConstIterator operator++(int) noexcept { ConstIterator t = *this; ++*this; return t; }
            SR_FORCE_INLINE ConstIterator& operator--() noexcept { m_p = Prev(m_p, m_nil); return *this; }
            ConstIterator operator--(int) noexcept { ConstIterator t = *this; --*this; return t; }

            SR_NODISCARD bool operator==(const ConstIterator& o) const noexcept { return m_p == o.m_p; }
            SR_NODISCARD bool operator!=(const ConstIterator& o) const noexcept { return m_p != o.m_p; }

        private:
            const Node* m_p   = nullptr;
            const Node* m_nil = nullptr;
        };

        using iterator       = Iterator;
        using const_iterator = ConstIterator;

    public:
        Map() { AllocNil(); }
        Map(std::nullptr_t) = delete;

        explicit Map(IAllocator* pAllocator)
            : m_allocator(pAllocator)
        { AllocNil(); }

        Map(std::initializer_list<ValueType> init);
        Map(const Map& other);
        Map(Map&& other) noexcept;
        ~Map();

    public:
        Map& operator=(const Map& other);
        Map& operator=(Map&& other) noexcept;
        Map& operator=(std::initializer_list<ValueType> init);

    public:
        // element access
        Value& operator[](const Key& key);
        Value& operator[](Key&& key);
        SR_NODISCARD Value& at(const Key& key);
        SR_NODISCARD const Value& at(const Key& key) const { return const_cast<Map*>(this)->at(key); }

        // lookup  — all in O(log n)
        SR_NODISCARD Iterator      find(const Key& key)        noexcept;
        SR_NODISCARD ConstIterator find(const Key& key)  const noexcept;
        SR_NODISCARD bool          contains(const Key& key) const noexcept;
        SR_NODISCARD uint32_t      count(const Key& key) const noexcept { return contains(key) ? 1 : 0; }
        SR_NODISCARD Iterator      lower_bound(const Key& key) noexcept;
        SR_NODISCARD ConstIterator lower_bound(const Key& key) const noexcept;
        SR_NODISCARD Iterator      upper_bound(const Key& key) noexcept;
        SR_NODISCARD ConstIterator upper_bound(const Key& key) const noexcept;

        // capacity
        SR_NODISCARD bool     empty()  const noexcept { return m_size == 0; }
        SR_NODISCARD SizeType size()   const noexcept { return m_size; }
        SR_NODISCARD IAllocator* GetAllocator() const noexcept { return m_allocator; }

        // iterators — begin() is O(1) via m_pMin cache
        SR_NODISCARD Iterator      begin()  noexcept { return { m_pMin, m_nil }; }
        SR_NODISCARD Iterator      end()    noexcept { return { m_nil,  m_nil }; }
        SR_NODISCARD ConstIterator begin()  const noexcept { return { m_pMin, m_nil }; }
        SR_NODISCARD ConstIterator end()    const noexcept { return { m_nil,  m_nil }; }
        SR_NODISCARD ConstIterator cbegin() const noexcept { return begin(); }
        SR_NODISCARD ConstIterator cend()   const noexcept { return end(); }

        // modifiers
        Pair<Iterator, bool> insert(const ValueType& kv);
        Pair<Iterator, bool> insert(ValueType&& kv);
        Pair<Iterator, bool> insert(const Key& key, const Value& value) { return insert(ValueType{ key, value }); }
        Pair<Iterator, bool> insert(Key&& key, Value&& value)           { return insert(ValueType{ std::move(key), std::move(value) }); }

        template<typename... Args>
        Pair<Iterator, bool> emplace(Args&&... args);

        bool     erase(const Key& key) noexcept;
        Iterator erase(Iterator pos)   noexcept;

        void clear() noexcept;
        void swap(Map& other) noexcept;

        template<typename Predicate> void erase_if(Predicate pred) noexcept;
        template<typename Predicate> bool any_of(Predicate pred) const noexcept;

        SR_NODISCARD Map DetachAllocator() const;

    private:
        // ---- memory ---------------------------------------------------------
        SR_NODISCARD void* RawAlloc(SizeType n) const;
        void               RawFree(void* p, SizeType n) const;

        SR_NODISCARD Node* NewNode() const { return static_cast<Node*>(RawAlloc(sizeof(Node))); }
        void               DelNode(Node* p) const { RawFree(p, sizeof(Node)); }

        // Allocate/free the sentinel nil node (no kv construction needed)
        void AllocNil();
        void FreeNil();

        // ---- tree helpers ---------------------------------------------------
        SR_NODISCARD SR_FORCE_INLINE Node* Root()    const noexcept { return m_nil->pParent; }
        SR_FORCE_INLINE void               SetRoot(Node* r) noexcept { m_nil->pParent = r; r->pParent = m_nil; }

        SR_FORCE_INLINE void RotLeft(Node* x)  noexcept;
        SR_FORCE_INLINE void RotRight(Node* x) noexcept;
        void InsertFixup(Node* z) noexcept;
        void EraseFixup(Node* x, Node* xp) noexcept;
        void Transplant(Node* u, Node* v) noexcept;

        // first node where !(node->key < key)
        SR_NODISCARD Node* LB(const Key& key) const noexcept;

        // Core insert — returns {existingOrNew, inserted}
        Pair<Node*, bool> InsertAt(Node* z) noexcept;

        // Subtree copy / destroy
        Node* CloneSubtree(Node* src, Node* srcNil, Node* parent);
        void  FreeSubtree(Node* x) noexcept;

        // In-order next / prev (static helpers used by iterators)
        SR_NODISCARD SR_FORCE_INLINE static Node* Next(Node* x, Node* nil) noexcept {
            if (x->pRight != nil) {
                x = x->pRight;
                while (x->pLeft != nil) x = x->pLeft;
                return x;
            }
            Node* p = x->pParent;
            while (p != nil && x == p->pRight) { x = p; p = p->pParent; }
            return p;
        }
        SR_NODISCARD SR_FORCE_INLINE static const Node* Next(const Node* x, const Node* nil) noexcept {
            if (x->pRight != nil) {
                x = x->pRight;
                while (x->pLeft != nil) x = x->pLeft;
                return x;
            }
            const Node* p = x->pParent;
            while (p != nil && x == p->pRight) { x = p; p = p->pParent; }
            return p;
        }
        SR_NODISCARD SR_FORCE_INLINE static Node* Prev(Node* x, Node* nil) noexcept {
            if (x == nil) {
                // --end() → maximum of entire tree
                // nil->pParent == root; walk right spine
                x = nil->pParent;
                while (x->pRight != nil) x = x->pRight;
                return x;
            }
            if (x->pLeft != nil) {
                x = x->pLeft;
                while (x->pRight != nil) x = x->pRight;
                return x;
            }
            Node* p = x->pParent;
            while (p != nil && x == p->pLeft) { x = p; p = p->pParent; }
            return p;
        }
        SR_NODISCARD SR_FORCE_INLINE static const Node* Prev(const Node* x, const Node* nil) noexcept {
            return Prev(const_cast<Node*>(x), const_cast<Node*>(nil));
        }

        static Node* TreeMin(Node* x, Node* nil) noexcept {
            while (x->pLeft != nil) x = x->pLeft;
            return x;
        }

    private:
        Node*       m_nil      = nullptr;   // heap-allocated sentinel; nil->pParent == root
        Node*       m_pMin     = nullptr;   // leftmost node (begin() cache)
        SizeType    m_size     = 0;
        IAllocator* m_allocator = nullptr;
        Compare     m_cmp{};
    };

    // =========================================================================
    // Implementation
    // =========================================================================

    template<typename Key, typename Value, typename Compare> template<typename Predicate> bool Map<Key, Value, Compare>::any_of(Predicate pred) const noexcept {
        for (auto it = begin(); it != end(); ++it) {
            if (pred(*it)) {
                return true;
            }
        }
        return false;
    }

    template<typename Key, typename Value, typename Compare> template<typename Predicate> void Map<Key, Value, Compare>::erase_if(Predicate pred) noexcept {
        for (auto it = begin(); it != end(); ) {
            if (pred(*it)) {
                it = erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // ---- memory -------------------------------------------------------------

    template<typename K, typename V, typename C>
    void* Map<K,V,C>::RawAlloc(SizeType n) const {
        void* p = m_allocator ? m_allocator->Allocate(n) : SRMalloc(n);
        if (!p) { SRHalt("Map: allocation of {} bytes failed!", n); }
        return p;
    }

    template<typename K, typename V, typename C>
    void Map<K,V,C>::RawFree(void* p, SizeType n) const {
        if (m_allocator) m_allocator->Free(p, n);
        else             SRFree(p);
    }

    template<typename K, typename V, typename C>
    void Map<K,V,C>::AllocNil() {
        // Allocate raw memory — no constructor for kv, just set pointers / color
        m_nil = static_cast<Node*>(RawAlloc(sizeof(Node)));
        m_nil->pLeft   = m_nil;
        m_nil->pRight  = m_nil;
        m_nil->pParent = m_nil;  // nil->pParent == root (empty tree → root == nil)
        m_nil->color   = Color::Black;
        m_pMin = m_nil;
    }

    template<typename K, typename V, typename C>
    void Map<K,V,C>::FreeNil() {
        // Do NOT call destructor — kv was never constructed in nil
        RawFree(m_nil, sizeof(Node));
        m_nil = nullptr;
    }

    // ---- rotations ----------------------------------------------------------

    template<typename K, typename V, typename C>
    SR_FORCE_INLINE void Map<K,V,C>::RotLeft(Node* x) noexcept {
        Node* y   = x->pRight;
        x->pRight = y->pLeft;
        if (y->pLeft != m_nil) y->pLeft->pParent = x;
        y->pParent = x->pParent;
        if (x == Root())              SetRoot(y);
        else if (x == x->pParent->pLeft) x->pParent->pLeft  = y;
        else                              x->pParent->pRight = y;
        y->pLeft   = x;
        x->pParent = y;
    }

    template<typename K, typename V, typename C>
    SR_FORCE_INLINE void Map<K,V,C>::RotRight(Node* x) noexcept {
        Node* y  = x->pLeft;
        x->pLeft = y->pRight;
        if (y->pRight != m_nil) y->pRight->pParent = x;
        y->pParent = x->pParent;
        if (x == Root())               SetRoot(y);
        else if (x == x->pParent->pRight) x->pParent->pRight = y;
        else                               x->pParent->pLeft  = y;
        y->pRight  = x;
        x->pParent = y;
    }

    // ---- InsertFixup --------------------------------------------------------

    template<typename K, typename V, typename C>
    void Map<K,V,C>::InsertFixup(Node* z) noexcept {
        while (z->pParent->color == Color::Red) {
            Node* gp = z->pParent->pParent;
            if (z->pParent == gp->pLeft) {
                Node* u = gp->pRight;
                if (u->color == Color::Red) {                   // Case 1: uncle red
                    z->pParent->color = Color::Black;
                    u->color          = Color::Black;
                    gp->color         = Color::Red;
                    z = gp;
                }
                else {
                    if (z == z->pParent->pRight) {              // Case 2 → Case 3
                        z = z->pParent;
                        RotLeft(z);
                    }
                    z->pParent->color = Color::Black;            // Case 3
                    z->pParent->pParent->color = Color::Red;
                    RotRight(z->pParent->pParent);
                }
            }
            else {
                Node* u = gp->pLeft;
                if (u->color == Color::Red) {
                    z->pParent->color = Color::Black;
                    u->color          = Color::Black;
                    gp->color         = Color::Red;
                    z = gp;
                }
                else {
                    if (z == z->pParent->pLeft) {
                        z = z->pParent;
                        RotRight(z);
                    }
                    z->pParent->color = Color::Black;
                    z->pParent->pParent->color = Color::Red;
                    RotLeft(z->pParent->pParent);
                }
            }
        }
        Root()->color = Color::Black;
    }

    // ---- Transplant ---------------------------------------------------------

    template<typename K, typename V, typename C>
    void Map<K,V,C>::Transplant(Node* u, Node* v) noexcept {
        if (u == Root())                   SetRoot(v);
        else if (u == u->pParent->pLeft)   u->pParent->pLeft  = v;
        else                               u->pParent->pRight = v;
        // Do not overwrite m_nil->pParent (which tracks root) when v is the sentinel.
        if (v != m_nil) v->pParent = u->pParent;
    }

    // ---- EraseFixup ---------------------------------------------------------

    template<typename K, typename V, typename C>
    void Map<K,V,C>::EraseFixup(Node* x, Node* xp) noexcept {
        while (x != Root() && x->color == Color::Black) {
            if (x == xp->pLeft) {
                Node* w = xp->pRight;
                if (w->color == Color::Red) {
                    w->color   = Color::Black;
                    xp->color  = Color::Red;
                    RotLeft(xp);
                    w = xp->pRight;
                }
                if (w->pLeft->color == Color::Black && w->pRight->color == Color::Black) {
                    w->color = Color::Red;
                    x = xp; xp = x->pParent;
                }
                else {
                    if (w->pRight->color == Color::Black) {
                        w->pLeft->color = Color::Black;
                        w->color        = Color::Red;
                        RotRight(w);
                        w = xp->pRight;
                    }
                    w->color         = xp->color;
                    xp->color        = Color::Black;
                    w->pRight->color = Color::Black;
                    RotLeft(xp);
                    x = Root();
                }
            }
            else {
                Node* w = xp->pLeft;
                if (w->color == Color::Red) {
                    w->color   = Color::Black;
                    xp->color  = Color::Red;
                    RotRight(xp);
                    w = xp->pLeft;
                }
                if (w->pRight->color == Color::Black && w->pLeft->color == Color::Black) {
                    w->color = Color::Red;
                    x = xp; xp = x->pParent;
                }
                else {
                    if (w->pLeft->color == Color::Black) {
                        w->pRight->color = Color::Black;
                        w->color         = Color::Red;
                        RotLeft(w);
                        w = xp->pLeft;
                    }
                    w->color        = xp->color;
                    xp->color       = Color::Black;
                    w->pLeft->color = Color::Black;
                    RotRight(xp);
                    x = Root();
                }
            }
        }
        x->color = Color::Black;
    }

    // ---- LB -----------------------------------------------------------------

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Node* Map<K,V,C>::LB(const K& key) const noexcept {
        Node* x  = Root();
        Node* lb = m_nil;
        while (x != m_nil) {
            if (!m_cmp(x->key(), key)) { lb = x; x = x->pLeft; }
            else                        { x = x->pRight; }
        }
        return lb;
    }

    // ---- InsertAt -----------------------------------------------------------

    template<typename K, typename V, typename C>
    Pair<typename Map<K,V,C>::Node*, bool> Map<K,V,C>::InsertAt(Node* z) noexcept {
        Node* parent = m_nil;
        Node* x      = Root();
        while (x != m_nil) {
            parent = x;
            if      (m_cmp(z->key(), x->key())) x = x->pLeft;
            else if (m_cmp(x->key(), z->key())) x = x->pRight;
            else                                 return Pair<Node*, bool>(x, false);   // duplicate
        }
        z->pLeft = z->pRight = m_nil;
        z->color = Color::Red;
        if (parent == m_nil)                     SetRoot(z);
        else if (m_cmp(z->key(), parent->key())) parent->pLeft  = z;
        else                                     parent->pRight = z;
        z->pParent = parent;
        InsertFixup(z);
        ++m_size;
        if (m_pMin == m_nil || m_cmp(z->key(), m_pMin->key())) m_pMin = z;
        return Pair<Node*, bool>(z, true);
    }

    // ---- public insert ------------------------------------------------------

    template<typename K, typename V, typename C>
    Pair<typename Map<K,V,C>::Iterator, bool>
    Map<K,V,C>::insert(const ValueType& kv) {
        Node* z = NewNode();
        new (z) Node(kv.first, kv.second);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    template<typename K, typename V, typename C>
    Pair<typename Map<K,V,C>::Iterator, bool>
    Map<K,V,C>::insert(ValueType&& kv) {
        Node* z = NewNode();
        new (z) Node(std::move(const_cast<K&>(kv.first)), std::move(kv.second));
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    template<typename K, typename V, typename C>
    template<typename... Args>
    Pair<typename Map<K,V,C>::Iterator, bool>
    Map<K,V,C>::emplace(Args&&... args) {
        Node* z = NewNode();
        new (z) Node(std::forward<Args>(args)...);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    // ---- operator[] — inlined insert path for speed -------------------------

    template<typename K, typename V, typename C>
    V& Map<K,V,C>::operator[](const K& key) {
        Node* parent = m_nil;
        Node* x      = Root();
        while (x != m_nil) {
            parent = x;
            if      (m_cmp(key, x->key())) x = x->pLeft;
            else if (m_cmp(x->key(), key)) x = x->pRight;
            else                           return x->kv.second;
        }
        Node* z = NewNode();
        new (z) Node(key);
        z->pLeft = z->pRight = m_nil; z->color = Color::Red;
        if (parent == m_nil)                     SetRoot(z);
        else if (m_cmp(z->key(), parent->key())) parent->pLeft  = z;
        else                                     parent->pRight = z;
        z->pParent = parent;
        InsertFixup(z);
        ++m_size;
        if (m_pMin == m_nil || m_cmp(z->key(), m_pMin->key())) m_pMin = z;
        return z->kv.second;
    }

    template<typename K, typename V, typename C>
    V& Map<K,V,C>::operator[](K&& key) {
        Node* parent = m_nil;
        Node* x      = Root();
        while (x != m_nil) {
            parent = x;
            if      (m_cmp(key, x->key())) x = x->pLeft;
            else if (m_cmp(x->key(), key)) x = x->pRight;
            else                           return x->kv.second;
        }
        Node* z = NewNode();
        new (z) Node(std::move(key), V{});
        z->pLeft = z->pRight = m_nil; z->color = Color::Red;
        if (parent == m_nil)                     SetRoot(z);
        else if (m_cmp(z->key(), parent->key())) parent->pLeft  = z;
        else                                     parent->pRight = z;
        z->pParent = parent;
        InsertFixup(z);
        ++m_size;
        if (m_pMin == m_nil || m_cmp(z->key(), m_pMin->key())) m_pMin = z;
        return z->kv.second;
    }

    // ---- at -----------------------------------------------------------------

    template<typename K, typename V, typename C>
    V& Map<K,V,C>::at(const K& key) {
        Node* x = LB(key);
        if (x == m_nil || m_cmp(key, x->key())) {
            SRHalt("Map::at() : key not found!");
            static V dummy{};
            return dummy;
        }
        return x->kv.second;
    }

    // ---- find / contains / lower_bound / upper_bound ------------------------

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Iterator Map<K,V,C>::find(const K& key) noexcept {
        Node* x = LB(key);
        return (x != m_nil && !m_cmp(key, x->key())) ? Iterator{ x, m_nil } : end();
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::ConstIterator Map<K,V,C>::find(const K& key) const noexcept {
        Node* x = LB(key);
        return (x != m_nil && !m_cmp(key, x->key())) ? ConstIterator{ x, m_nil } : end();
    }

    template<typename K, typename V, typename C>
    bool Map<K,V,C>::contains(const K& key) const noexcept {
        Node* x = LB(key);
        return x != m_nil && !m_cmp(key, x->key());
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Iterator Map<K,V,C>::lower_bound(const K& key) noexcept {
        return { LB(key), m_nil };
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::ConstIterator Map<K,V,C>::lower_bound(const K& key) const noexcept {
        return { LB(key), m_nil };
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Iterator Map<K,V,C>::upper_bound(const K& key) noexcept {
        Node* x = Root(), *ub = m_nil;
        while (x != m_nil) {
            if (m_cmp(key, x->key())) { ub = x; x = x->pLeft; }
            else                       { x = x->pRight; }
        }
        return { ub, m_nil };
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::ConstIterator Map<K,V,C>::upper_bound(const K& key) const noexcept {
        Node* x = Root(), *ub = m_nil;
        while (x != m_nil) {
            if (m_cmp(key, x->key())) { ub = x; x = x->pLeft; }
            else                       { x = x->pRight; }
        }
        return { ub, m_nil };
    }

    // ---- erase --------------------------------------------------------------

    template<typename K, typename V, typename C>
    bool Map<K,V,C>::erase(const K& key) noexcept {
        Node* z = LB(key);
        if (z == m_nil || m_cmp(key, z->key())) return false;

        if (z == m_pMin) m_pMin = Next(z, m_nil);

        Node* y  = z;
        Color yc = y->color;
        Node* x  = nullptr;
        Node* xp = nullptr;

        if (z->pLeft == m_nil) {
            x = z->pRight; xp = z->pParent;
            Transplant(z, z->pRight);
        }
        else if (z->pRight == m_nil) {
            x = z->pLeft; xp = z->pParent;
            Transplant(z, z->pLeft);
        }
        else {
            y  = TreeMin(z->pRight, m_nil);
            yc = y->color;
            x  = y->pRight;
            xp = (y->pParent == z) ? y : y->pParent;
            if (y->pParent != z) {
                Transplant(y, y->pRight);
                y->pRight = z->pRight;
                y->pRight->pParent = y;
            }
            Transplant(z, y);
            y->pLeft = z->pLeft;
            y->pLeft->pParent = y;
            y->color = z->color;
        }

        z->~Node();
        DelNode(z);
        --m_size;

        if (yc == Color::Black) EraseFixup(x, xp);

        if (m_size == 0) m_pMin = m_nil;

        return true;
    }

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Iterator Map<K,V,C>::erase(Iterator pos) noexcept {
        Node* node = pos.GetNode();
        if (node == m_nil) return end();
        Iterator nxt = pos; ++nxt;
        erase(node->key());
        return nxt;
    }

    // ---- clear / destroy ----------------------------------------------------

    template<typename K, typename V, typename C>
    void Map<K,V,C>::FreeSubtree(Node* x) noexcept {
        // Iterative left-spine traversal to keep stack depth minimal
        while (x != m_nil) {
            FreeSubtree(x->pRight);
            Node* left = x->pLeft;
            x->~Node();
            DelNode(x);
            x = left;
        }
    }

    template<typename K, typename V, typename C>
    void Map<K,V,C>::clear() noexcept {
        FreeSubtree(Root());
        m_nil->pParent = m_nil;
        m_pMin = m_nil;
        m_size = 0;
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>::~Map() {
        if (m_nil) {
            FreeSubtree(Root());
            FreeNil();
        }
    }

    // ---- CloneSubtree -------------------------------------------------------

    template<typename K, typename V, typename C>
    typename Map<K,V,C>::Node*
    Map<K,V,C>::CloneSubtree(Node* src, Node* srcNil, Node* parent) {
        if (src == srcNil) return m_nil;
        Node* dst = NewNode();
        new (dst) Node(src->kv.first, src->kv.second);
        dst->color   = src->color;
        dst->pParent = parent;
        dst->pLeft   = CloneSubtree(src->pLeft,  srcNil, dst);
        dst->pRight  = CloneSubtree(src->pRight, srcNil, dst);
        return dst;
    }

    // ---- constructors / assignment ------------------------------------------

    template<typename K, typename V, typename C>
    Map<K,V,C>::Map(std::initializer_list<ValueType> init) {
        AllocNil();
        for (auto& kv : init) insert(kv);
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>::Map(const Map& other) {
        m_allocator = other.m_allocator;
        AllocNil();
        if (other.m_size > 0) {
            Node* root = CloneSubtree(other.Root(), other.m_nil, m_nil);
            m_nil->pParent = root;
            m_pMin = TreeMin(root, m_nil);
        }
        m_size = other.m_size;
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>::Map(Map&& other) noexcept {
        // steal other's nil pointer; other gets a fresh one
        m_nil      = other.m_nil;
        m_pMin     = other.m_pMin;
        m_size     = other.m_size;
        m_allocator = other.m_allocator;
        m_cmp      = std::move(other.m_cmp);

        // leave other in a valid empty state
        other.m_nil = nullptr;
        other.AllocNil();
        other.m_pMin = other.m_nil;
        other.m_size = 0;
        other.m_allocator = nullptr;
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>& Map<K,V,C>::operator=(const Map& other) {
        if (this != &other) { Map tmp(other); swap(tmp); }
        return *this;
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>& Map<K,V,C>::operator=(Map&& other) noexcept {
        if (this != &other) { Map tmp(std::move(other)); swap(tmp); }
        return *this;
    }

    template<typename K, typename V, typename C>
    Map<K,V,C>& Map<K,V,C>::operator=(std::initializer_list<ValueType> init) {
        Map tmp(init);
        swap(tmp);
        return *this;
    }

    // ---- swap ---------------------------------------------------------------
    // Each Map owns its nil pointer, so swap just exchanges all pointers.

    template<typename K, typename V, typename C>
    void Map<K,V,C>::swap(Map& other) noexcept {
        std::swap(m_nil,       other.m_nil);
        std::swap(m_pMin,      other.m_pMin);
        std::swap(m_size,      other.m_size);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_cmp,       other.m_cmp);
    }

    // ---- DetachAllocator ----------------------------------------------------

    template<typename K, typename V, typename C>
    Map<K,V,C> Map<K,V,C>::DetachAllocator() const {
        Map result;
        for (auto& [k, v] : *this) result.insert(k, v);
        return result;
    }
}

#endif //SR_COMMON_MAP_H
