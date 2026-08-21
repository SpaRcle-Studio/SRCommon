//
// Created by Monika on 23.07.2026.
//

#if !defined(SR_COMMON_SET_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_COMMON_SET_H

#include <Utils/Common/AssertFwd.h>

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

    private:
        enum class Color : uint8_t { Red, Black };

        struct Node {
            T      value;
            Node*  pLeft   = nullptr;
            Node*  pRight  = nullptr;
            Node*  pParent = nullptr;
            Color  color   = Color::Red;

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
            using difference_type = std::ptrdiff_t;

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
        // lookup
        SR_NODISCARD Iterator      find(const T& value)        noexcept;
        SR_NODISCARD ConstIterator find(const T& value)  const noexcept;
        SR_NODISCARD bool          contains(const T& value) const noexcept;
        SR_NODISCARD uint32_t      count(const T& value) const noexcept { return contains(value) ? 1 : 0; }
        SR_NODISCARD Iterator      lower_bound(const T& value) noexcept;
        SR_NODISCARD ConstIterator lower_bound(const T& value) const noexcept;
        SR_NODISCARD Iterator      upper_bound(const T& value) noexcept;
        SR_NODISCARD ConstIterator upper_bound(const T& value) const noexcept;

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
        std::pair<Iterator, bool> insert(const T& value);
        std::pair<Iterator, bool> insert(T&& value);
        void insert(Iterator first, Iterator last);

        template<typename... Args>
        std::pair<Iterator, bool> emplace(Args&&... args);

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
        SR_NODISCARD SR_FORCE_INLINE Node* Root()    const noexcept { return m_nil->pParent; }
        SR_FORCE_INLINE void               SetRoot(Node* r) noexcept { m_nil->pParent = r; r->pParent = m_nil; }

        SR_FORCE_INLINE void RotLeft(Node* x)  noexcept;
        SR_FORCE_INLINE void RotRight(Node* x) noexcept;
        void InsertFixup(Node* z) noexcept;
        void EraseFixup(Node* x, Node* xp) noexcept;
        void Transplant(Node* u, Node* v) noexcept;

        SR_NODISCARD Node* LB(const T& value) const noexcept;
        std::pair<Node*, bool> InsertAt(Node* z) noexcept;

        Node* CloneSubtree(Node* src, Node* srcNil, Node* parent);
        void  FreeSubtree(Node* x) noexcept;

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
        SR_NODISCARD SR_FORCE_INLINE static Node* Prev(Node* x, Node* nil) noexcept {
            if (x == nil) {
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

        static Node* TreeMin(Node* x, Node* nil) noexcept {
            while (x->pLeft != nil) x = x->pLeft;
            return x;
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
        m_nil->pLeft   = m_nil;
        m_nil->pRight  = m_nil;
        m_nil->pParent = m_nil;
        m_nil->color   = Color::Black;
        m_pMin = m_nil;
    }

    template<typename T, typename C>
    void Set<T,C>::FreeNil() {
        if (m_nil) {
            RawFree(m_nil, sizeof(Node));
            m_nil = nullptr;
        }
    }

    // ---- rotations ----------------------------------------------------------

    template<typename T, typename C>
    SR_FORCE_INLINE void Set<T,C>::RotLeft(Node* x) noexcept {
        Node* y   = x->pRight;
        x->pRight = y->pLeft;
        if (y->pLeft != m_nil) y->pLeft->pParent = x;
        y->pParent = x->pParent;
        if (x == Root())               SetRoot(y);
        else if (x == x->pParent->pLeft)  x->pParent->pLeft  = y;
        else                              x->pParent->pRight = y;
        y->pLeft   = x;
        x->pParent = y;
    }

    template<typename T, typename C>
    SR_FORCE_INLINE void Set<T,C>::RotRight(Node* x) noexcept {
        Node* y  = x->pLeft;
        x->pLeft = y->pRight;
        if (y->pRight != m_nil) y->pRight->pParent = x;
        y->pParent = x->pParent;
        if (x == Root())                SetRoot(y);
        else if (x == x->pParent->pRight) x->pParent->pRight = y;
        else                              x->pParent->pLeft  = y;
        y->pRight  = x;
        x->pParent = y;
    }

    // ---- InsertFixup --------------------------------------------------------

    template<typename T, typename C>
    void Set<T,C>::InsertFixup(Node* z) noexcept {
        while (z->pParent->color == Color::Red) {
            Node* gp = z->pParent->pParent;
            if (z->pParent == gp->pLeft) {
                Node* u = gp->pRight;
                if (u->color == Color::Red) {
                    z->pParent->color = Color::Black;
                    u->color          = Color::Black;
                    gp->color         = Color::Red;
                    z = gp;
                }
                else {
                    if (z == z->pParent->pRight) { z = z->pParent; RotLeft(z); }
                    z->pParent->color          = Color::Black;
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
                    if (z == z->pParent->pLeft) { z = z->pParent; RotRight(z); }
                    z->pParent->color          = Color::Black;
                    z->pParent->pParent->color = Color::Red;
                    RotLeft(z->pParent->pParent);
                }
            }
        }
        Root()->color = Color::Black;
    }

    // ---- Transplant ---------------------------------------------------------

    template<typename T, typename C>
    void Set<T,C>::Transplant(Node* u, Node* v) noexcept {
        if (u == Root())                  SetRoot(v);
        else if (u == u->pParent->pLeft)  u->pParent->pLeft  = v;
        else                              u->pParent->pRight = v;
        // Do not overwrite m_nil->pParent (which tracks root) when v is the sentinel.
        if (v != m_nil) v->pParent = u->pParent;
    }

    // ---- EraseFixup ---------------------------------------------------------

    template<typename T, typename C>
    void Set<T,C>::EraseFixup(Node* x, Node* xp) noexcept {
        while (x != Root() && x->color == Color::Black) {
            if (x == xp->pLeft) {
                Node* w = xp->pRight;
                if (w->color == Color::Red) {
                    w->color  = Color::Black;
                    xp->color = Color::Red;
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
                    w->color  = Color::Black;
                    xp->color = Color::Red;
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

    // ---- LB / InsertAt ------------------------------------------------------

    template<typename T, typename C>
    typename Set<T,C>::Node* Set<T,C>::LB(const T& value) const noexcept {
        Node* x = Root(), *lb = m_nil;
        while (x != m_nil) {
            if (!m_cmp(x->value, value)) { lb = x; x = x->pLeft; }
            else                          { x = x->pRight; }
        }
        return lb;
    }

    template<typename T, typename C>
    std::pair<typename Set<T,C>::Node*, bool> Set<T,C>::InsertAt(Node* z) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            AllocNil();
        }
        Node* parent = m_nil, *x = Root();
        while (x != m_nil) {
            parent = x;
            if      (m_cmp(z->value, x->value)) x = x->pLeft;
            else if (m_cmp(x->value, z->value)) x = x->pRight;
            else                                 return { x, false };
        }
        z->pLeft = z->pRight = m_nil;
        z->color = Color::Red;
        if (parent == m_nil)                       SetRoot(z);
        else if (m_cmp(z->value, parent->value))   parent->pLeft  = z;
        else                                       parent->pRight = z;
        z->pParent = parent;
        InsertFixup(z);
        ++m_size;
        if (m_pMin == m_nil || m_cmp(z->value, m_pMin->value)) m_pMin = z;
        return { z, true };
    }

    // ---- public insert / emplace --------------------------------------------

    template<typename T, typename C>
    std::pair<typename Set<T,C>::Iterator, bool> Set<T,C>::insert(const T& value) {
        Node* z = NewNode();
        new (z) Node(value);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return { { n, m_nil }, ok };
    }

    template<typename T, typename C>
    std::pair<typename Set<T,C>::Iterator, bool> Set<T,C>::insert(T&& value) {
        Node* z = NewNode();
        new (z) Node(std::move(value));
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return { { n, m_nil }, ok };
    }

    template<typename T, typename C>
    template<typename... Args>
    std::pair<typename Set<T,C>::Iterator, bool> Set<T,C>::emplace(Args&&... args) {
        Node* z = NewNode();
        new (z) Node(std::forward<Args>(args)...);
        auto [n, ok] = InsertAt(z);
        if (!ok) { z->~Node(); DelNode(z); }
        return { { n, m_nil }, ok };
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
        Node* x = Root(), *ub = m_nil;
        while (x != m_nil) {
            if (m_cmp(value, x->value)) { ub = x; x = x->pLeft; }
            else                         { x = x->pRight; }
        }
        return { ub, m_nil };
    }

    template<typename T, typename C>
    typename Set<T,C>::ConstIterator Set<T,C>::upper_bound(const T& value) const noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return end();
        }
        Node* x = Root(), *ub = m_nil;
        while (x != m_nil) {
            if (m_cmp(value, x->value)) { ub = x; x = x->pLeft; }
            else                         { x = x->pRight; }
        }
        return { ub, m_nil };
    }

    // ---- erase --------------------------------------------------------------

    template<typename T, typename C>
    bool Set<T,C>::erase(const T& value) noexcept {
        if (!m_nil) SR_UNLIKELY_ATTRIBUTE {
            return false;
        }
        Node* z = LB(value);
        if (z == m_nil || m_cmp(value, z->value)) return false;

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
        while (x != m_nil) {
            FreeSubtree(x->pRight);
            Node* left = x->pLeft;
            x->~Node();
            DelNode(x);
            x = left;
        }
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
    Set<T,C>::CloneSubtree(Node* src, Node* srcNil, Node* parent) {
        if (src == srcNil) return m_nil;
        Node* dst = NewNode();
        new (dst) Node(src->value);
        dst->color   = src->color;
        dst->pParent = parent;
        dst->pLeft   = CloneSubtree(src->pLeft,  srcNil, dst);
        dst->pRight  = CloneSubtree(src->pRight, srcNil, dst);
        return dst;
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
