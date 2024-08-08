#if MLN_RENDER_BACKEND_OPENGL
#include <mbgl/test/util.hpp>

#include <unordered_set>

#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/headless_backend.hpp>
#include <mbgl/gl/context.hpp>

using namespace mbgl;

namespace {

constexpr auto oneMB = 1024 * 1024;

constexpr gl::Texture2DDesc makeDescOneMB() {
    return {{1024, 1024}, gfx::TexturePixelType::Luminance, gfx::TextureChannelDataType::UnsignedByte};
}

} // namespace

TEST(ResourcePool, TexturePool) {
    gl::HeadlessBackend backend{{32, 32}};

    gl::Context context{backend};
    EXPECT_TRUE(context.empty());

    auto& pool = context.getTexturePool();
    EXPECT_LE(oneMB, pool.maxStorage());

    EXPECT_EQ(pool.usedStorage(), 0);
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), 0);
    EXPECT_FALSE(pool.isUsed(3));
    EXPECT_FALSE(pool.isUnused(3));
    EXPECT_FALSE(pool.isPooled(3));

    auto id = pool.alloc(makeDescOneMB());
    EXPECT_EQ(pool.usedStorage(), oneMB);
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), oneMB);
    EXPECT_TRUE(pool.isUsed(id));
    EXPECT_FALSE(pool.isUnused(id));
    EXPECT_TRUE(pool.isPooled(id));

    pool.release(id);
    EXPECT_EQ(pool.usedStorage(), 0u);
    EXPECT_EQ(pool.unusedStorage(), oneMB);
    EXPECT_EQ(pool.storage(), oneMB);
    EXPECT_FALSE(pool.isUsed(id));
    EXPECT_TRUE(pool.isUnused(id));
    EXPECT_TRUE(pool.isPooled(id));

    auto reused_id = pool.alloc(makeDescOneMB());
    EXPECT_EQ(reused_id, id);
    EXPECT_EQ(pool.usedStorage(), oneMB);
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), oneMB);
    EXPECT_TRUE(pool.isUsed(id));
    EXPECT_FALSE(pool.isUnused(id));
    EXPECT_TRUE(pool.isPooled(id));

    std::unordered_set<gl::TextureID> ids = {id};
    while (pool.storage() <= pool.maxStorage()) {
        auto new_id = pool.alloc(makeDescOneMB());
        EXPECT_TRUE(ids.find(new_id) != ids.end());
        EXPECT_EQ(pool.usedStorage(), oneMB * ids.size());
        EXPECT_EQ(pool.unusedStorage(), 0);
        EXPECT_EQ(pool.storage(), pool.usedStorage());
        EXPECT_TRUE(pool.isUsed(new_id));
        EXPECT_FALSE(pool.isUnused(new_id));
        EXPECT_TRUE(pool.isPooled(new_id));
        ids.insert(new_id);
    }

    EXPECT_LE(pool.maxStorage(), pool.usedStorage());
    EXPECT_EQ(pool.usedStorage(), oneMB * ids.size());
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), pool.usedStorage());
    EXPECT_TRUE(pool.isUsed(id));
    EXPECT_FALSE(pool.isUnused(id));
    EXPECT_TRUE(pool.isPooled(id));

    pool.release(id);
    ids.erase(id);
    EXPECT_GE(pool.maxStorage(), pool.usedStorage());
    EXPECT_EQ(pool.usedStorage(), oneMB * ids.size());
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), pool.usedStorage());
    EXPECT_FALSE(pool.isUsed(id));
    EXPECT_FALSE(pool.isUnused(id));
    EXPECT_FALSE(pool.isPooled(id));

    context.reduceMemoryUsage();
    EXPECT_EQ(pool.usedStorage(), 0);
    EXPECT_EQ(pool.unusedStorage(), 0);
    EXPECT_EQ(pool.storage(), 0);
    EXPECT_FALSE(pool.isUsed(id));
    EXPECT_FALSE(pool.isUnused(id));
    EXPECT_FALSE(pool.isPooled(id));
    EXPECT_TRUE(context.empty());
}

#endif
