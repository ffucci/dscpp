#include "shared_ptr/shared_ptr.hpp"
#include "shared_ptr/weak_ptr.hpp"

#include <gtest/gtest.h>

namespace {
    struct Tracked {
        static inline int constructions = 0;
        static inline int destructions = 0;

        int value{0};

        explicit Tracked(int v = 0) : value(v) { ++constructions; }

        ~Tracked() { ++destructions; }

        static void reset() {
            constructions = 0;
            destructions = 0;
        }
    };
} // namespace

TEST(WeakPtr, DefaultConstructedIsEmpty) {
    cpplearn::memory::WeakPtr<Tracked> weak;

    EXPECT_EQ(weak.use_count(), 0);
    EXPECT_TRUE(weak.expired());

    auto locked = weak.lock();
    EXPECT_EQ(locked.get(), nullptr);
    EXPECT_EQ(locked.use_count(), 0);
    EXPECT_FALSE(locked);
}

TEST(WeakPtr, ConstructedFromSharedPtrObservesManagedObject) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> owner(new Tracked(7));
    cpplearn::memory::WeakPtr<Tracked> weak(owner);

    EXPECT_EQ(owner.use_count(), 1);
    EXPECT_EQ(weak.use_count(), 1);
    EXPECT_FALSE(weak.expired());

    auto locked = weak.lock();
    EXPECT_EQ(locked.get(), owner.get());
    EXPECT_EQ(locked.use_count(), 2);
    EXPECT_EQ(locked->value, 7);
    EXPECT_EQ(Tracked::destructions, 0);

    locked.reset();

    EXPECT_EQ(owner.use_count(), 1);
    EXPECT_EQ(weak.use_count(), 1);
}

TEST(WeakPtr, CopyConstructionSharesObservation) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> owner(new Tracked(3));
    cpplearn::memory::WeakPtr<Tracked> first(owner);
    cpplearn::memory::WeakPtr<Tracked> second(first);

    EXPECT_EQ(owner.use_count(), 1);
    EXPECT_EQ(first.use_count(), 1);
    EXPECT_EQ(second.use_count(), 1);
    EXPECT_FALSE(first.expired());
    EXPECT_FALSE(second.expired());

    auto locked = second.lock();
    EXPECT_EQ(locked.get(), owner.get());
    EXPECT_EQ(locked.use_count(), 2);
}

TEST(WeakPtr, MoveConstructionTransfersObservation) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> owner(new Tracked(11));
    cpplearn::memory::WeakPtr<Tracked> first(owner);
    cpplearn::memory::WeakPtr<Tracked> second(std::move(first));

    EXPECT_EQ(first.use_count(), 0);
    EXPECT_TRUE(first.expired());
    EXPECT_EQ(second.use_count(), 1);
    EXPECT_FALSE(second.expired());

    auto locked = second.lock();
    EXPECT_EQ(locked.get(), owner.get());
    EXPECT_EQ(locked->value, 11);
}

TEST(WeakPtr, ResetDetachesWeakObservation) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> owner(new Tracked(5));
    cpplearn::memory::WeakPtr<Tracked> weak(owner);

    weak.reset();

    EXPECT_EQ(owner.use_count(), 1);
    EXPECT_EQ(weak.use_count(), 0);
    EXPECT_TRUE(weak.expired());

    auto locked = weak.lock();
    EXPECT_EQ(locked.get(), nullptr);
    EXPECT_FALSE(locked);
}

TEST(WeakPtr, AssignmentRebindsToAnotherSharedOwner) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> first_owner(new Tracked(1));
    cpplearn::memory::SharedPtr<Tracked> second_owner(new Tracked(2));
    cpplearn::memory::WeakPtr<Tracked> weak(first_owner);

    weak = second_owner;

    EXPECT_EQ(first_owner.use_count(), 1);
    EXPECT_EQ(second_owner.use_count(), 1);
    EXPECT_EQ(weak.use_count(), 1);
    EXPECT_FALSE(weak.expired());

    auto locked = weak.lock();
    EXPECT_EQ(locked.get(), second_owner.get());
    EXPECT_EQ(locked->value, 2);
}

TEST(WeakPtr, ExpiresAfterLastSharedOwnerReleasesObject) {
    Tracked::reset();

    cpplearn::memory::WeakPtr<Tracked> weak;
    {
        cpplearn::memory::SharedPtr<Tracked> owner(new Tracked(21));
        weak = owner;

        EXPECT_EQ(owner.use_count(), 1);
        EXPECT_EQ(weak.use_count(), 1);
        EXPECT_FALSE(weak.expired());
    }

    EXPECT_EQ(Tracked::constructions, 1);
    EXPECT_EQ(Tracked::destructions, 1);
    EXPECT_EQ(weak.use_count(), 0);
    EXPECT_TRUE(weak.expired());

    auto locked = weak.lock();
    EXPECT_EQ(locked.get(), nullptr);
    EXPECT_EQ(locked.use_count(), 0);
    EXPECT_FALSE(locked);
}
