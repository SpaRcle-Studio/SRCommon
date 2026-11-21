//
// Created by Codex on 21.11.2026.
//

#ifndef SR_ENGINE_UTILS_TYPES_MAP_CORE_H
#define SR_ENGINE_UTILS_TYPES_MAP_CORE_H

#include <Utils/Types/Pair.h>
#include <Utils/Common/AssertFwd.h>
#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class IAllocator;

    enum class RbTreeColor : uint8_t {
        Red = 0,
        Black = 1
    };

    struct SR_COMMON_DLL_API RbTreeNodeHeader {
        RbTreeNodeHeader* pLeft = nullptr;
        RbTreeNodeHeader* pRight = nullptr;
        RbTreeNodeHeader* pParent = nullptr;
        RbTreeColor color = RbTreeColor::Red;
        uint8_t unused[sizeof(void*) - sizeof(RbTreeColor)] = { 0 };
    };

    struct SR_COMMON_DLL_API ErasedTreeMapLayout {
        RbTreeNodeHeader* pNil = nullptr;
        RbTreeNodeHeader* pMin = nullptr;
        SizeType size = 0;
        IAllocator* pAllocator = nullptr;
        void* pUnused = nullptr;
    };

    static_assert(sizeof(RbTreeNodeHeader) == 4 * sizeof(void*));
    static_assert(alignof(RbTreeNodeHeader) == alignof(void*));
    static_assert(sizeof(SizeType) == sizeof(size_t));
    static_assert(sizeof(ErasedTreeMapLayout) == 4 * sizeof(void*) + sizeof(size_t));
    static_assert(alignof(ErasedTreeMapLayout) == alignof(void*));

    constexpr SizeType AlignUp(SizeType value, SizeType alignment) noexcept {
        return alignment <= 1 ? value : (value + alignment - 1) & ~(alignment - 1);
    }

    constexpr SizeType CalculatePairSecondOffset(SizeType keySize, SizeType valueAlign) noexcept {
        return AlignUp(keySize, valueAlign);
    }

    constexpr SizeType CalculatePairAlign(SizeType keyAlign, SizeType valueAlign) noexcept {
        return keyAlign > valueAlign ? keyAlign : valueAlign;
    }

    constexpr SizeType CalculatePairSize(SizeType keySize, SizeType keyAlign, SizeType valueSize, SizeType valueAlign) noexcept {
        const SizeType pairAlign = CalculatePairAlign(keyAlign, valueAlign);
        const SizeType secondOffset = CalculatePairSecondOffset(keySize, valueAlign);
        return AlignUp(secondOffset + valueSize, pairAlign);
    }

    constexpr SizeType CalculateMapNodeKeyOffset(SizeType keyAlign, SizeType valueAlign) noexcept {
        const SizeType pairAlign = CalculatePairAlign(keyAlign, valueAlign);
        return AlignUp(sizeof(RbTreeNodeHeader), pairAlign);
    }

    constexpr SizeType CalculateMapNodeValueOffset(SizeType keySize, SizeType keyAlign, SizeType valueAlign) noexcept {
        return CalculateMapNodeKeyOffset(keyAlign, valueAlign) + CalculatePairSecondOffset(keySize, valueAlign);
    }

    constexpr SizeType CalculateMapNodeSize(SizeType keySize, SizeType keyAlign, SizeType valueSize, SizeType valueAlign) noexcept {
        const SizeType pairSize = CalculatePairSize(keySize, keyAlign, valueSize, valueAlign);
        const SizeType pairAlign = CalculatePairAlign(keyAlign, valueAlign);
        const SizeType nodeAlign = pairAlign > alignof(RbTreeNodeHeader) ? pairAlign : alignof(RbTreeNodeHeader);
        const SizeType pairOffset = AlignUp(sizeof(RbTreeNodeHeader), pairAlign);
        return AlignUp(pairOffset + pairSize, nodeAlign);
    }

    constexpr SizeType CalculateSetNodeValueOffset(SizeType valueAlign) noexcept {
        return AlignUp(sizeof(RbTreeNodeHeader), valueAlign);
    }

    constexpr SizeType CalculateSetNodeSize(SizeType valueSize, SizeType valueAlign) noexcept {
        const SizeType nodeAlign = valueAlign > alignof(RbTreeNodeHeader) ? valueAlign : alignof(RbTreeNodeHeader);
        const SizeType valueOffset = AlignUp(sizeof(RbTreeNodeHeader), valueAlign);
        return AlignUp(valueOffset + valueSize, nodeAlign);
    }

    namespace MapCore {
        SR_FORCE_INLINE void InitNil(RbTreeNodeHeader* nil) noexcept {
            nil->pLeft = nil;
            nil->pRight = nil;
            nil->pParent = nil;
            nil->color = RbTreeColor::Black;
        }

        SR_FORCE_INLINE RbTreeNodeHeader* Root(const RbTreeNodeHeader* nil) noexcept {
            return nil->pParent;
        }

        SR_FORCE_INLINE void SetRoot(RbTreeNodeHeader* r, RbTreeNodeHeader* nil) noexcept {
            nil->pParent = r;
            r->pParent = nil;
        }

        SR_FORCE_INLINE RbTreeNodeHeader* TreeMin(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
            while (x->pLeft != nil) x = x->pLeft;
            return x;
        }

        SR_FORCE_INLINE const RbTreeNodeHeader* TreeMin(const RbTreeNodeHeader* x, const RbTreeNodeHeader* nil) noexcept {
            while (x->pLeft != nil) x = x->pLeft;
            return x;
        }

        SR_FORCE_INLINE RbTreeNodeHeader* TreeMax(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
            while (x->pRight != nil) x = x->pRight;
            return x;
        }

        SR_FORCE_INLINE const RbTreeNodeHeader* TreeMax(const RbTreeNodeHeader* x, const RbTreeNodeHeader* nil) noexcept {
            while (x->pRight != nil) x = x->pRight;
            return x;
        }

        SR_FORCE_INLINE RbTreeNodeHeader* Next(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
            if (x->pRight != nil) {
                x = x->pRight;
                while (x->pLeft != nil) x = x->pLeft;
                return x;
            }
            RbTreeNodeHeader* p = x->pParent;
            while (p != nil && x == p->pRight) { x = p; p = p->pParent; }
            return p;
        }

        SR_FORCE_INLINE const RbTreeNodeHeader* Next(const RbTreeNodeHeader* x, const RbTreeNodeHeader* nil) noexcept {
            if (x->pRight != nil) {
                x = x->pRight;
                while (x->pLeft != nil) x = x->pLeft;
                return x;
            }
            const RbTreeNodeHeader* p = x->pParent;
            while (p != nil && x == p->pRight) { x = p; p = p->pParent; }
            return p;
        }

        SR_FORCE_INLINE RbTreeNodeHeader* Prev(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
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
            RbTreeNodeHeader* p = x->pParent;
            while (p != nil && x == p->pLeft) { x = p; p = p->pParent; }
            return p;
        }

        SR_FORCE_INLINE const RbTreeNodeHeader* Prev(const RbTreeNodeHeader* x, const RbTreeNodeHeader* nil) noexcept {
            return Prev(const_cast<RbTreeNodeHeader*>(x), const_cast<RbTreeNodeHeader*>(nil));
        }

        SR_FORCE_INLINE void RotLeft(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
            RbTreeNodeHeader* y = x->pRight;
            x->pRight = y->pLeft;
            if (y->pLeft != nil) y->pLeft->pParent = x;
            y->pParent = x->pParent;
            if (x == nil->pParent) SetRoot(y, nil);
            else if (x == x->pParent->pLeft) x->pParent->pLeft = y;
            else x->pParent->pRight = y;
            y->pLeft = x;
            x->pParent = y;
        }

        SR_FORCE_INLINE void RotRight(RbTreeNodeHeader* x, RbTreeNodeHeader* nil) noexcept {
            RbTreeNodeHeader* y = x->pLeft;
            x->pLeft = y->pRight;
            if (y->pRight != nil) y->pRight->pParent = x;
            y->pParent = x->pParent;
            if (x == nil->pParent) SetRoot(y, nil);
            else if (x == x->pParent->pRight) x->pParent->pRight = y;
            else x->pParent->pLeft = y;
            y->pRight = x;
            x->pParent = y;
        }

        inline void InsertFixup(RbTreeNodeHeader* z, RbTreeNodeHeader* nil) noexcept {
            while (z->pParent->color == RbTreeColor::Red) {
                RbTreeNodeHeader* gp = z->pParent->pParent;
                if (z->pParent == gp->pLeft) {
                    RbTreeNodeHeader* u = gp->pRight;
                    if (u->color == RbTreeColor::Red) {
                        z->pParent->color = RbTreeColor::Black;
                        u->color = RbTreeColor::Black;
                        gp->color = RbTreeColor::Red;
                        z = gp;
                    }
                    else {
                        if (z == z->pParent->pRight) {
                            z = z->pParent;
                            RotLeft(z, nil);
                        }
                        z->pParent->color = RbTreeColor::Black;
                        z->pParent->pParent->color = RbTreeColor::Red;
                        RotRight(z->pParent->pParent, nil);
                    }
                }
                else {
                    RbTreeNodeHeader* u = gp->pLeft;
                    if (u->color == RbTreeColor::Red) {
                        z->pParent->color = RbTreeColor::Black;
                        u->color = RbTreeColor::Black;
                        gp->color = RbTreeColor::Red;
                        z = gp;
                    }
                    else {
                        if (z == z->pParent->pLeft) {
                            z = z->pParent;
                            RotRight(z, nil);
                        }
                        z->pParent->color = RbTreeColor::Black;
                        z->pParent->pParent->color = RbTreeColor::Red;
                        RotLeft(z->pParent->pParent, nil);
                    }
                }
            }
            nil->pParent->color = RbTreeColor::Black;
        }

        SR_FORCE_INLINE void Transplant(RbTreeNodeHeader* u, RbTreeNodeHeader* v, RbTreeNodeHeader* nil) noexcept {
            if (u == nil->pParent) SetRoot(v, nil);
            else if (u == u->pParent->pLeft) u->pParent->pLeft = v;
            else u->pParent->pRight = v;
            if (v != nil) v->pParent = u->pParent;
        }

        inline void EraseFixup(RbTreeNodeHeader* x, RbTreeNodeHeader* xp, RbTreeNodeHeader* nil) noexcept {
            while (x != nil->pParent && x->color == RbTreeColor::Black) {
                if (x == xp->pLeft) {
                    RbTreeNodeHeader* w = xp->pRight;
                    if (w->color == RbTreeColor::Red) {
                        w->color = RbTreeColor::Black;
                        xp->color = RbTreeColor::Red;
                        RotLeft(xp, nil);
                        w = xp->pRight;
                    }
                    if (w->pLeft->color == RbTreeColor::Black && w->pRight->color == RbTreeColor::Black) {
                        w->color = RbTreeColor::Red;
                        x = xp; xp = x->pParent;
                    }
                    else {
                        if (w->pRight->color == RbTreeColor::Black) {
                            w->pLeft->color = RbTreeColor::Black;
                            w->color = RbTreeColor::Red;
                            RotRight(w, nil);
                            w = xp->pRight;
                        }
                        w->color = xp->color;
                        xp->color = RbTreeColor::Black;
                        w->pRight->color = RbTreeColor::Black;
                        RotLeft(xp, nil);
                        x = nil->pParent;
                    }
                }
                else {
                    RbTreeNodeHeader* w = xp->pLeft;
                    if (w->color == RbTreeColor::Red) {
                        w->color = RbTreeColor::Black;
                        xp->color = RbTreeColor::Red;
                        RotRight(xp, nil);
                        w = xp->pLeft;
                    }
                    if (w->pRight->color == RbTreeColor::Black && w->pLeft->color == RbTreeColor::Black) {
                        w->color = RbTreeColor::Red;
                        x = xp; xp = x->pParent;
                    }
                    else {
                        if (w->pLeft->color == RbTreeColor::Black) {
                            w->pRight->color = RbTreeColor::Black;
                            w->color = RbTreeColor::Red;
                            RotLeft(w, nil);
                            w = xp->pLeft;
                        }
                        w->color = xp->color;
                        xp->color = RbTreeColor::Black;
                        w->pLeft->color = RbTreeColor::Black;
                        RotRight(xp, nil);
                        x = nil->pParent;
                    }
                }
            }
            x->color = RbTreeColor::Black;
        }

        template<typename NodeHeaderType>
        inline void UnlinkNode(NodeHeaderType*& pMin, NodeHeaderType* z, NodeHeaderType* nil) noexcept {
            if (z == pMin) pMin = static_cast<NodeHeaderType*>(Next(z, nil));

            RbTreeNodeHeader* y = z;
            RbTreeColor yc = y->color;
            RbTreeNodeHeader* x = nullptr;
            RbTreeNodeHeader* xp = nullptr;

            if (z->pLeft == nil) {
                x = z->pRight;
                xp = z->pParent;
                Transplant(z, z->pRight, nil);
            }
            else if (z->pRight == nil) {
                x = z->pLeft;
                xp = z->pParent;
                Transplant(z, z->pLeft, nil);
            }
            else {
                y = TreeMin(z->pRight, nil);
                yc = y->color;
                x = y->pRight;
                xp = (y->pParent == z) ? y : y->pParent;
                if (y->pParent != z) {
                    Transplant(y, y->pRight, nil);
                    y->pRight = z->pRight;
                    y->pRight->pParent = y;
                }
                Transplant(z, y, nil);
                y->pLeft = z->pLeft;
                y->pLeft->pParent = y;
                y->color = z->color;
            }

            if (yc == RbTreeColor::Black) {
                EraseFixup(x, xp, nil);
            }
        }

        template<typename KeyType, typename NodeHeaderType, typename KeyExtract, typename Compare>
        SR_FORCE_INLINE NodeHeaderType* LowerBound(NodeHeaderType* nil, const KeyType& key, KeyExtract&& keyExtract, Compare&& cmp) noexcept {
            NodeHeaderType* x = static_cast<NodeHeaderType*>(nil->pParent);
            NodeHeaderType* lb = nil;
            while (x != nil) {
                if (!cmp(keyExtract(x), key)) {
                    lb = x;
                    x = static_cast<NodeHeaderType*>(x->pLeft);
                }
                else {
                    x = static_cast<NodeHeaderType*>(x->pRight);
                }
            }
            return lb;
        }

        template<typename KeyType, typename NodeHeaderType, typename KeyExtract, typename Compare>
        SR_FORCE_INLINE const NodeHeaderType* LowerBound(const NodeHeaderType* nil, const KeyType& key, KeyExtract&& keyExtract, Compare&& cmp) noexcept {
            const NodeHeaderType* x = static_cast<const NodeHeaderType*>(nil->pParent);
            const NodeHeaderType* lb = nil;
            while (x != nil) {
                if (!cmp(keyExtract(x), key)) {
                    lb = x;
                    x = static_cast<const NodeHeaderType*>(x->pLeft);
                }
                else {
                    x = static_cast<const NodeHeaderType*>(x->pRight);
                }
            }
            return lb;
        }

        template<typename KeyType, typename NodeHeaderType, typename KeyExtract, typename Compare>
        SR_FORCE_INLINE NodeHeaderType* UpperBound(NodeHeaderType* nil, const KeyType& key, KeyExtract&& keyExtract, Compare&& cmp) noexcept {
            NodeHeaderType* x = static_cast<NodeHeaderType*>(nil->pParent);
            NodeHeaderType* ub = nil;
            while (x != nil) {
                if (cmp(key, keyExtract(x))) {
                    ub = x;
                    x = static_cast<NodeHeaderType*>(x->pLeft);
                }
                else {
                    x = static_cast<NodeHeaderType*>(x->pRight);
                }
            }
            return ub;
        }

        template<typename KeyType, typename NodeHeaderType, typename KeyExtract, typename Compare>
        SR_FORCE_INLINE const NodeHeaderType* UpperBound(const NodeHeaderType* nil, const KeyType& key, KeyExtract&& keyExtract, Compare&& cmp) noexcept {
            const NodeHeaderType* x = static_cast<const NodeHeaderType*>(nil->pParent);
            const NodeHeaderType* ub = nil;
            while (x != nil) {
                if (cmp(key, keyExtract(x))) {
                    ub = x;
                    x = static_cast<const NodeHeaderType*>(x->pLeft);
                }
                else {
                    x = static_cast<const NodeHeaderType*>(x->pRight);
                }
            }
            return ub;
        }

        template<typename NodeHeaderType, typename KeyExtract, typename Compare>
        Pair<NodeHeaderType*, bool> InsertNode(NodeHeaderType*& pMin, SizeType& size, NodeHeaderType* z, NodeHeaderType* nil, KeyExtract&& keyExtract, Compare&& cmp) noexcept {
            NodeHeaderType* parent = nil;
            NodeHeaderType* x = static_cast<NodeHeaderType*>(nil->pParent);
            auto&& zKey = keyExtract(z);
            while (x != nil) {
                parent = x;
                auto&& xKey = keyExtract(x);
                if (cmp(zKey, xKey)) x = static_cast<NodeHeaderType*>(x->pLeft);
                else if (cmp(xKey, zKey)) x = static_cast<NodeHeaderType*>(x->pRight);
                else return Pair<NodeHeaderType*, bool>(x, false);
            }
            z->pLeft = z->pRight = nil;
            z->color = RbTreeColor::Red;
            if (parent == nil) {
                SetRoot(z, nil);
            }
            else if (cmp(zKey, keyExtract(parent))) {
                parent->pLeft = z;
            }
            else {
                parent->pRight = z;
            }
            z->pParent = parent;
            InsertFixup(z, nil);
            ++size;
            if (pMin == nil || cmp(zKey, keyExtract(pMin))) {
                pMin = z;
            }
            return Pair<NodeHeaderType*, bool>(z, true);
        }

        template<typename NodeHeaderType, typename DestroyFn>
        inline void DestroySubtree(NodeHeaderType* x, NodeHeaderType* nil, DestroyFn&& destroy) noexcept {
            while (x != nil) {
                DestroySubtree(static_cast<NodeHeaderType*>(x->pRight), nil, destroy);
                NodeHeaderType* left = static_cast<NodeHeaderType*>(x->pLeft);
                destroy(x);
                x = left;
            }
        }

        template<typename NodeHeaderType, typename CloneFn>
        NodeHeaderType* CloneSubtree(const NodeHeaderType* src, const NodeHeaderType* srcNil, NodeHeaderType* dstNil, NodeHeaderType* parent, CloneFn&& clone) {
            if (src == srcNil) return dstNil;
            NodeHeaderType* dst = clone(src);
            dst->color   = src->color;
            dst->pParent = parent;
            dst->pLeft   = CloneSubtree(static_cast<const NodeHeaderType*>(src->pLeft),  srcNil, dstNil, dst, clone);
            dst->pRight  = CloneSubtree(static_cast<const NodeHeaderType*>(src->pRight), srcNil, dstNil, dst, clone);
            return dst;
        }
    }
}

#endif // SR_ENGINE_UTILS_TYPES_MAP_CORE_H
