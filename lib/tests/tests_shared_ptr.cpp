#include "shared_ptr/shared_ptr.hpp"

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

    struct CountingDeleter {
        int *calls{nullptr};

        void operator()(Tracked *ptr) const noexcept {
            if (calls != nullptr) {
                ++(*calls);
            }
            delete ptr;
        }
    };
} // namespace

TEST(SharedPtr, DefaultConstructedIsEmpty) {
    cpplearn::memory::SharedPtr<Tracked> ptr;

    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.use_count(), 0);
    EXPECT_FALSE(ptr);
}

TEST(SharedPtr, RawPointerConstructorOwnsObject) {
    Tracked::reset(); {
        cpplearn::memory::SharedPtr<Tracked> ptr(new Tracked(7));
        EXPECT_NE(ptr.get(), nullptr);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(ptr->value, 7);
        EXPECT_EQ((*ptr).value, 7);
    }

    EXPECT_EQ(Tracked::constructions, 1);
    EXPECT_EQ(Tracked::destructions, 1);
}

TEST(SharedPtr, CopyIncrementsUseCount) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> first(new Tracked(3));
    cpplearn::memory::SharedPtr<Tracked> second(first);

    EXPECT_EQ(first.get(), second.get());
    EXPECT_EQ(first.use_count(), 2);
    EXPECT_EQ(second.use_count(), 2);
    EXPECT_EQ(Tracked::destructions, 0);
}

TEST(SharedPtr, MoveTransfersOwnership) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> first(new Tracked(11));
    const Tracked *raw = first.get();

    cpplearn::memory::SharedPtr<Tracked> second(std::move(first));

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(first.use_count(), 0);
    EXPECT_EQ(second.get(), raw);
    EXPECT_EQ(second.use_count(), 1);
}

TEST(SharedPtr, ResetReleasesSharedOwnership) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> first(new Tracked(5));
    cpplearn::memory::SharedPtr<Tracked> second(first);

    first.reset();

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(first.use_count(), 0);
    EXPECT_EQ(second.use_count(), 1);
    EXPECT_EQ(Tracked::destructions, 0);

    second.reset();

    EXPECT_EQ(Tracked::destructions, 1);
}

TEST(SharedPtr, SelfAssignmentIsSafe) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> ptr(new Tracked(9));
    const Tracked *raw = ptr.get();

    ptr = ptr;

    EXPECT_EQ(ptr.get(), raw);
    EXPECT_EQ(ptr.use_count(), 1);
    EXPECT_EQ(Tracked::destructions, 0);
}

TEST(SharedPtr, CustomDeleterRunsOnce) {
    Tracked::reset();

    int delete_calls = 0; {
        cpplearn::memory::SharedPtr<Tracked> ptr(new Tracked(1), CountingDeleter{&delete_calls});
        EXPECT_EQ(ptr.use_count(), 1);
    }

    EXPECT_EQ(delete_calls, 1);
    EXPECT_EQ(Tracked::destructions, 1);
}

TEST(SharedPtr, UseCountIncrementsMultipleTimes) {
    Tracked::reset();

    cpplearn::memory::SharedPtr<Tracked> ptr(new Tracked(10));
    [[maybe_unused]] auto ptr2 = ptr; {
        [[maybe_unused]] auto ptr3 = ptr;
    }
    EXPECT_EQ(ptr.get(), ptr2.get());
    EXPECT_EQ(ptr.use_count(), 2);
}
