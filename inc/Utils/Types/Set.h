//
// Created by Monika on 23.07.2026.
//

#if !defined(SR_COMMON_SET_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_COMMON_SET_H

#include <Utils/Common/AssertFwd.h>
#include <Utils/Types/MapCore.h>
#include <Utils/Types/Pair.h>

namespace SR_UTILS_NS {
    // =========================================================================
    // Set — ordered unique-element container backed by a Red-Black tree.
    //
    // Same structural optimisations as Map:
    //   • Heap-allocated nil sentinel per instance (nil->pParent == root).
    //   • m_pMin cache for O(1) begin().
    //   • SR_FORCE_INLINE on hot paths.
    //   • Iterative left-spine destroy to minimise stack depth.
    //
    // Complexity: O(log n) insert / erase / find / lower_bound / upper_bound.
    // =========================================================================
    template<typename Compare, typename T, typename K>
    concept SetTransparentLookup = requires {
        typename Compare::is_transparent;
    } && requires(const Compare& cmp, const T& stored, const K& key) {
        { cmp(stored, key) } -> std::convertible_to<bool>;
        { cmp(key, stored) } -> std::convertible_to<bool>;
    };

    template<
        typename T,
        typename Compare = std::less<T>
    >
    class Set {
    public:
        using SizeType  = size_t;
        using ValueType = T;
        using KeyType = T;
        using value_type = T;
        using key_type = T;
        static constexpr bool key_only = true;

        static constexpr void ValidateErasedLayout() {
            using SetType = Set<T, Compare>;
            if constexpr (std::is_empty_v<Compare>) {
                static_assert(sizeof(Set<T, Compare>) == sizeof(ErasedTreeMapLayout));
                static_assert(alignof(Set<T, Compare>) == alignof(ErasedTreeMapLayout));
                static_assert(offsetof(SetType, m_nil) == offsetof(ErasedTreeMapLayout, pNil));
                static_assert(offsetof(SetType, m_pMin) == offsetof(ErasedTreeMapLayout, pMin));
                static_assert(offsetof(SetType, m_size) == offsetof(ErasedTreeMapLayout, size));
                static_assert(offsetof(SetType, m_allocator) == offsetof(ErasedTreeMapLayout, pAllocator));
            }
            static_assert(sizeof(Node) == CalculateSetNodeSize(sizeof(T), alignof(T)));
            static_assert(offsetof(Node, value) == CalculateSetNodeValueOffset(alignof(T)));
        }

    private:
        using Color = RbTreeColor;

        struct Node : public RbTreeNodeHeader {
            T value;

            template<typename U>
            explicit Node(U&& v) : value(std::forward<U>(v)) { }

            template<typename... Args>
            explicit Node(Args&&... args) : value(std::forward<Args>(args)...) { }
        };

    public:
        // ---- Iterator -------------------------------------------------------
        struct Iterator {
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = T;
            using reference         = const T&;
            using pointer           = const T*;
            using difference_type   = std::ptrdiff_t;

            Iterator prev() const noexcept { return Iterator(Prev(m_p, m_nil), m_nil); }
            Iterator next() const noexcept { return Iterator(Next(m_p, m_nil), m_nil); }

            Iterator() noexcept = default;
            Iterator(const Iterator&) noexcept = default;
            Iterator(Iterator&&) noexcept = default;
            Iterator& operator=(const Iterator&) noexcept = default;
            Iterator& operator=(Iterator&&) noexcept = default;
            Iterator(Node* p, Node* nil) noexcept : m_p(p), m_nil(nil) { }

            SR_NODISCARD const T& operator*()  const noexcept { return m_p->value; }
            SR_NODISCARD const T* operator->() const noexcept { return &m_p->value; }

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

        using ConstIterator  = Iterator;   // Set elements are always immutable
        using iterator       = Iterator;
        using const_iterator = ConstIterator;

    public:
        Set() { }
        Set(std::nullptr_t) = delete;

        explicit Set(IAllocator* pAllocator)
            : m_allocator(pAllocator)
        { }

        Set(std::initializer_list<T> init);
        Set(const Set& other);
        Set(IAllocator* pAllocator, const Set& other);
        Set(Set&& other) noexcept;
        ~Set();

    public:
        Set& operator=(const Set& other);
        Set& operator=(Set&& other) noexcept;
        Set& operator=(std::initializer_list<T> init);

        bool operator==(const Set& other) const noexcept {
            return m_size == other.m_size && std::equal(begin(), end(), other.begin());
        }

        bool operator!=(const Set& other) const noexcept { return !(*this == other); }

    public:
        // Heterogeneous lookup is enabled only for transparent comparators.
        // The comparator must compare T and K directly without allocating.

        // lookup
        SR_NODISCARD Iterator      find(const T& value)        noexcept;
        SR_NODISCARD ConstIterator find(const T& value)  const noexcept;
        SR_NODISCARD bool          contains(const T& value) const noexcept;
        SR_NODISCARD uint32_t      count(const T& value) const noexcept { return contains(value) ? 1 : 0; }
        SR_NODISCARD Iterator      lower_bound(const T& value) noexcept;
        SR_NODISCARD ConstIterator lower_bound(const T& value) const noexcept;
        SR_NODISCARD Iterator      upper_bound(const T& value) noexcept;
        SR_NODISCARD ConstIterator upper_bound(const T& value) const noexcept;

        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD Iterator find(const K& value) noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD ConstIterator find(const K& value) const noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD bool contains(const K& value) const noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD uint32_t count(const K& value) const noexcept { return contains(value) ? 1 : 0; }
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD Iterator lower_bound(const K& value) noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD ConstIterator lower_bound(const K& value) const noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD Iterator upper_bound(const K& value) noexcept;
        template<typename K> requires (SetTransparentLookup<Compare, T, K>)
        SR_NODISCARD ConstIterator upper_bound(const K& value) const noexcept;

        // capacity
        SR_NODISCARD bool     empty() const noexcept { return m_size == 0; }
        SR_NODISCARD SizeType size()  const noexcept { return m_size; }
        SR_NODISCARD IAllocator* GetAllocator() const noexcept { return m_allocator; }

        // iterators
        SR_NODISCARD Iterator      begin()  noexcept { return { m_pMin, m_nil }; }
        SR_NODISCARD Iterator      end()    noexcept { return { m_nil,  m_nil }; }
        SR_NODISCARD ConstIterator begin()  const noexcept { return { m_pMin, m_nil }; }
        SR_NODISCARD ConstIterator end()    const noexcept { return { m_nil,  m_nil }; }
        SR_NODISCARD ConstIterator cbegin() const noexcept { return begin(); }
        SR_NODISCARD ConstIterator cend()   const noexcept { return end(); }

        template<typename Predicate> SR_NODISCARD bool all_of(Predicate&& pred) const;
        template<typename Predicate> SR_NODISCARD bool any_of(Predicate&& pred) const;
        template<typename Predicate> SR_NODISCARD bool none_of(Predicate&& pred) const;

        // modifiers
        Pair<Iterator, bool> insert(const T& value);
        Pair<Iterator, bool> insert(T&& value);
        void insert(Iterator first, Iterator last);

        template<typename... Args>
        Pair<Iterator, bool> emplace(Args&&... args);

        bool     erase(const T& value) noexcept;
        Iterator erase(Iterator pos)   noexcept;

        void clear() noexcept;
        void swap(Set& other) noexcept;

        SR_NODISCARD Set DetachAllocator() const;

    private:
        // ---- memory ---------------------------------------------------------
        SR_NODISCARD void* RawAlloc(SizeType n) const;
        void               RawFree(void* p, SizeType n) const;

        SR_NODISCARD Node* NewNode() const { return static_cast<Node*>(RawAlloc(sizeof(Node))); }
        void               DelNode(Node* p) const { RawFree(p, sizeof(Node)); }

        void AllocNil();
        void FreeNil();

        // ---- tree helpers ---------------------------------------------------
        SR_NODISCARD SR_FORCE_INLINE Node* Root()    const noexcept { return static_cast<Node*>(m_nil->pParent); }
        SR_FORCE_INLINE void               SetRoot(Node* r) noexcept { MapCore::SetRoot(r, m_nil); }

        template<typename K>
        SR_NODISCARD Node* LB(const K& value) const noexcept;
        Pair<Node*, bool>  InsertAt(Node* z) noexcept;

        Node* CloneSubtree(const Node* src, const Node* srcNil, Node* parent);
        void  FreeSubtree(Node* x) noexcept;

        SR_NODISCARD SR_FORCE_INLINE static Node* Next(Node* x, Node* nil) noexcept {
            return static_cast<Node*>(MapCore::Next(x, nil));
        }
        SR_NODISCARD SR_FORCE_INLINE static const Node* Next(const Node* x, const Node* nil) noexcept {
            return static_cast<const Node*>(MapCore::Next(x, nil));
        }
        SR_NODISCARD SR_FORCE_INLINE static Node* Prev(Node* x, Node* nil) noexcept {
            return static_cast<Node*>(MapCore::Prev(x, nil));
        }
        SR_NODISCARD SR_FORCE_INLINE static const Node* Prev(const Node* x, const Node* nil) noexcept {
            return static_cast<const Node*>(MapCore::Prev(x, nil));
        }

        static Node* TreeMin(Node* x, Node* nil) noexcept {
            return static_cast<Node*>(MapCore::TreeMin(x, nil));
        }
        static const Node* TreeMin(const Node* x, const Node* nil) noexcept {
            return static_cast<const Node*>(MapCore::TreeMin(x, nil));
        }

    private:
        Node*       m_nil      = nullptr;
        Node*       m_pMin     = nullptr;
        SizeType    m_size     = 0;
        IAllocator* m_allocator = nullptr;
        Compare     m_cmp{};
    };

    // =========================================================================
    // Implementation
    // =========================================================================

    template<typename T, typename Compare> void Set<T, Compare>::insert(Set::Iterator first, Set::Iterator last) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    template<typename T, typename Compare> template<typename Predicate> bool Set<T, Compare>::none_of(Predicate&& pred) const {
        for (const auto& value : *this) {
            if (pred(value)) {
                return false;
            }
        }
        return true;
    }

    template<typename T, typename Compare> template<typename Predicate> bool Set<T, Compare>::any_of(Predicate&& pred) const {
        for (const auto& value : *this) {
            if (pred(value)) {
                return true;
            }
        }
        return false;
    }

    template<typename T, typename Compare> template<typename Predicate> bool Set<T, Compare>::all_of(Predicate&& pred) const {
        for (const auto& value : *this) {
            if (!pred(value)) {
                return false;
            }
        }
        return true;
    }

    template<typename T, typename C>
    void* Set<T,C>::RawAlloc(SizeType n) const {
        void* p = m_allocator ? m_allocator->Allocate(n) : SRMalloc(n);
        if (!p) { SRHalt("Set: allocation of {} bytes failed!", n); }
        return p;
    }

    template<typename T, typename C>
    void Set<T,C>::RawFree(void* p, SizeType n) const {
        if (m_allocator) m_allocator->Free(p, n);
        else             SRFree(p);
    }

    template<typename T, typename C>
    void Set<T,C>::AllocNil() {
        m_nil = static_cast<Node*>(RawAlloc(sizeof(Node)));
        MapCore::InitNil(m_nil);
        m_pMin = m_nil;
    }

    template<typename T, typename C>
    void Set<T,C>::FreeNil() {
        if (m_nil) {
            RawFree(m_nil, sizeof(Node));
            m_nil = nullptr;
        }
    }

    // ---- LB / InsertAt ------------------------------------------------------

    template<typename T, typename C>
    template<typename K>
    typename Set<T,C>::Node* Set<T,C>::LB(const K& value) const noexcept {
        return const_cast<Node*>(MapCore::LowerBound(m_nil, value, [](const Node* n) -> const T& { return n->value; }, m_cmp));
    }

    template<typename T, typename C>
    Pair<typename Set<T,C>::Node*, bool> Set<T,C>::InsertAt(Node* z) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            AllocNil();
        }
        return MapCore::InsertNode(m_pMin, m_size, z, m_nil, [](const Node* n) -> const T& { return n->value; }, m_cmp);
    }

    // ---- public insert / emplace --------------------------------------------

    template<typename T, typename C>
    Pair<typename Set<T,C>::Iterator, bool> Set<T,C>::insert(const T& value) {
        Node* z = NewNode();
        new (z) Node(value);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    template<typename T, typename C>
    Pair<typename Set<T,C>::Iterator, bool> Set<T,C>::insert(T&& value) {
        Node* z = NewNode();
        new (z) Node(std::move(value));
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    template<typename T, typename C>
    template<typename... Args>
    Pair<typename Set<T,C>::Iterator, bool> Set<T,C>::emplace(Args&&... args) {
        Node* z = NewNode();
        new (z) Node(std::forward<Args>(args)...);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return Pair<Iterator, bool>(Iterator(n, m_nil), ok);
    }

    // ---- find / contains / bounds -------------------------------------------

    template<typename T, typename C>
    typename Set<T,C>::Iterator Set<T,C>::find(const T& value) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        Node* x = LB(value);
        return (x != m_nil && !m_cmp(value, x->value)) ? Iterator{ x, m_nil } : end();
    }

    template<typename T, typename C>
    typename Set<T,C>::ConstIterator Set<T,C>::find(const T& value) const noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        Node* x = LB(value);
        return (x != m_nil && !m_cmp(value, x->value)) ? ConstIterator{ x, m_nil } : end();
    }

    template<typename T, typename C>
    bool Set<T,C>::contains(const T& value) const noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return false;
        }
        Node* x = LB(value);
        return x != m_nil && !m_cmp(value, x->value);
    }

    template<typename T, typename C>
    typename Set<T,C>::Iterator Set<T,C>::lower_bound(const T& value) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        return { LB(value), m_nil };
    }

    template<typename T, typename C>
    typename Set<T,C>::ConstIterator Set<T,C>::lower_bound(const T& value) const noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        return { LB(value), m_nil };
    }

    template<typename T, typename C>
    typename Set<T,C>::Iterator Set<T,C>::upper_bound(const T& value) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        return { const_cast<Node*>(MapCore::UpperBound(m_nil, value, [](const Node* n) -> const T& { return n->value; }, m_cmp)), m_nil };
    }

    template<typename T, typename C>
    typename Set<T,C>::ConstIterator Set<T,C>::upper_bound(const T& value) const noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        return { MapCore::UpperBound(m_nil, value, [](const Node* n) -> const T& { return n->value; }, m_cmp), m_nil };
    }

    // ---- transparent heterogeneous lookup -----------------------------------

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::Iterator Set<T,C>::find(const K& value) noexcept {
        if (!m_nil) return end();
        Node* x = LB(value);
        return (x != m_nil && !m_cmp(value, x->value)) ? Iterator{ x, m_nil } : end();
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::ConstIterator Set<T,C>::find(const K& value) const noexcept {
        if (!m_nil) return end();
        Node* x = LB(value);
        return (x != m_nil && !m_cmp(value, x->value)) ? ConstIterator{ x, m_nil } : end();
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    bool Set<T,C>::contains(const K& value) const noexcept {
        if (!m_nil) return false;
        Node* x = LB(value);
        return x != m_nil && !m_cmp(value, x->value);
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::Iterator Set<T,C>::lower_bound(const K& value) noexcept {
        if (!m_nil) return end();
        return { LB(value), m_nil };
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::ConstIterator Set<T,C>::lower_bound(const K& value) const noexcept {
        if (!m_nil) return end();
        return { LB(value), m_nil };
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::Iterator Set<T,C>::upper_bound(const K& value) noexcept {
        if (!m_nil) return end();
        return { const_cast<Node*>(MapCore::UpperBound(m_nil, value, [](const Node* n) -> const T& { return n->value; }, m_cmp)), m_nil };
    }

    template<typename T, typename C>
    template<typename K> requires (SetTransparentLookup<C, T, K>)
    typename Set<T,C>::ConstIterator Set<T,C>::upper_bound(const K& value) const noexcept {
        if (!m_nil) return end();
        return { MapCore::UpperBound(m_nil, value, [](const Node* n) -> const T& { return n->value; }, m_cmp), m_nil };
    }

    // ---- erase --------------------------------------------------------------

    template<typename T, typename C>
    bool Set<T,C>::erase(const T& value) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return false;
        }
        Node* z = LB(value);
        if (z == m_nil || m_cmp(value, z->value)) return false;

        MapCore::UnlinkNode(m_pMin, z, m_nil);

        z->~Node();
        DelNode(z);
        --m_size;

        if (m_size == 0) m_pMin = m_nil;

        return true;
    }

    template<typename T, typename C>
    typename Set<T,C>::Iterator Set<T,C>::erase(Iterator pos) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        Node* node = pos.GetNode();
        if (node == m_nil) return end();
        Iterator nxt = pos; ++nxt;
        erase(node->value);
        return nxt;
    }

    // ---- clear / destroy ----------------------------------------------------

    template<typename T, typename C>
    void Set<T,C>::FreeSubtree(Node* x) noexcept {
        MapCore::DestroySubtree(x, m_nil, [this](Node* node) {
            node->~Node();
            DelNode(node);
        });
    }

    template<typename T, typename C>
    void Set<T,C>::clear() noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return;
        }
        FreeSubtree(Root());
        m_nil->pParent = m_nil;
        m_pMin = m_nil;
        m_size = 0;
    }

    template<typename T, typename C>
    Set<T,C>::~Set() {
        if (m_nil) {
            FreeSubtree(Root());
            FreeNil();
        }
    }

    // ---- CloneSubtree -------------------------------------------------------

    template<typename T, typename C>
    typename Set<T,C>::Node*
    Set<T,C>::CloneSubtree(const Node* src, const Node* srcNil, Node* parent) {
        return MapCore::CloneSubtree(src, srcNil, m_nil, parent, [this](const Node* s) {
            Node* dst = NewNode();
            new (dst) Node(s->value);
            return dst;
        });
    }

    // ---- constructors / assignment ------------------------------------------

    template<typename T, typename C>
    Set<T,C>::Set(std::initializer_list<T> init) {
        for (auto& v : init) insert(v);
    }

    template<typename T, typename C>
    Set<T,C>::Set(const Set& other) {
        m_allocator = other.m_allocator;
        if (other.m_size > 0) {
            AllocNil();
            Node* root = CloneSubtree(other.Root(), other.m_nil, m_nil);
            m_nil->pParent = root;
            m_pMin = TreeMin(root, m_nil);
        }
        m_size = other.m_size;
    }

    template<typename T, typename C>
    Set<T,C>::Set(IAllocator* pAllocator, const Set& other) {
        m_allocator = pAllocator;
        if (other.m_size > 0) {
            AllocNil();
            Node* root = CloneSubtree(other.Root(), other.m_nil, m_nil);
            m_nil->pParent = root;
            m_pMin = TreeMin(root, m_nil);
        }
        m_size = other.m_size;
    }

    template<typename T, typename C>
    Set<T,C>::Set(Set&& other) noexcept {
        m_nil       = other.m_nil;
        m_pMin      = other.m_pMin;
        m_size      = other.m_size;
        m_allocator = other.m_allocator;
        m_cmp       = std::move(other.m_cmp);

        other.m_nil = nullptr;
        other.m_pMin = other.m_nil;
        other.m_size = 0;
        other.m_allocator = nullptr;
    }

    template<typename T, typename C>
    Set<T,C>& Set<T,C>::operator=(const Set& other) {
        if (this != &other) { Set tmp(other); swap(tmp); }
        return *this;
    }

    template<typename T, typename C>
    Set<T,C>& Set<T,C>::operator=(Set&& other) noexcept {
        if (this != &other) { Set tmp(std::move(other)); swap(tmp); }
        return *this;
    }

    template<typename T, typename C>
    Set<T,C>& Set<T,C>::operator=(std::initializer_list<T> init) {
        Set tmp(init);
        swap(tmp);
        return *this;
    }

    template<typename T, typename C>
    void Set<T,C>::swap(Set& other) noexcept {
        std::swap(m_nil,       other.m_nil);
        std::swap(m_pMin,      other.m_pMin);
        std::swap(m_size,      other.m_size);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_cmp,       other.m_cmp);
    }

    template<typename T, typename C>
    Set<T,C> Set<T,C>::DetachAllocator() const {
        Set result;
        for (auto& v : *this) result.insert(v);
        return result;
    }
}

#endif //SR_COMMON_SET_H
