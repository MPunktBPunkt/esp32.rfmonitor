#include <unity.h>
#include "rf/RfScore.h"

static RfScoreFeatures baseFeat() {
    RfScoreFeatures f;
    f.ownRssi = -48;
    f.rssiJitter = 2;
    f.densityProxy = 12;
    f.pressureProxy = 10;
    f.occupancyProxy = 24;
    f.stability = 100;
    f.connected = true;
    f.measurementQuality = 100;
    return f;
}

void test_v2_lab_like_is_high() {
    auto r = RfScore::computeV2(baseFeat());
    TEST_ASSERT_EQUAL_UINT8(2, r.version);
    TEST_ASSERT_TRUE(r.score >= 90);
}

void test_v1_lab_like_is_high() {
    auto r = RfScore::computeV1(baseFeat());
    TEST_ASSERT_EQUAL_UINT8(1, r.version);
    TEST_ASSERT_TRUE(r.score >= 90);
}

void test_rssi_drop_lowers_score() {
    auto good = baseFeat();
    auto bad = baseFeat();
    bad.ownRssi = -70;
    auto g = RfScore::computeV2(good);
    auto b = RfScore::computeV2(bad);
    TEST_ASSERT_TRUE(b.score < g.score);
    TEST_ASSERT_TRUE(b.parts.ownLink > g.parts.ownLink);
}

void test_disconnect_penalty() {
    auto on = baseFeat();
    auto off = baseFeat();
    off.connected = false;
    auto a = RfScore::computeV2(on);
    auto b = RfScore::computeV2(off);
    TEST_ASSERT_TRUE(b.score < a.score);
    TEST_ASSERT_EQUAL_UINT8(20, b.parts.disconnect);
    TEST_ASSERT_EQUAL_UINT8(25, RfScore::computeV1(off).parts.disconnect);
}

void test_density_pressure_lower_score() {
    auto calm = baseFeat();
    auto dense = baseFeat();
    dense.densityProxy = 80;
    dense.pressureProxy = 80;
    auto a = RfScore::computeV2(calm);
    auto b = RfScore::computeV2(dense);
    TEST_ASSERT_TRUE(b.score < a.score);
    TEST_ASSERT_TRUE(b.parts.density > a.parts.density);
    TEST_ASSERT_TRUE(b.parts.pressure > a.parts.pressure);
}

void test_occupancy_v2_only() {
    auto f = baseFeat();
    f.occupancyProxy = 100;
    auto v1 = RfScore::computeV1(f);
    auto v2 = RfScore::computeV2(f);
    TEST_ASSERT_EQUAL_UINT8(0, v1.parts.occupancy);
    TEST_ASSERT_TRUE(v2.parts.occupancy > 0);
}

void test_quality_gate() {
    auto ok = baseFeat();
    auto weak = baseFeat();
    weak.measurementQuality = 30;
    auto a = RfScore::computeV2(ok);
    auto b = RfScore::computeV2(weak);
    TEST_ASSERT_EQUAL_UINT8(0, a.parts.qualityGate);
    TEST_ASSERT_EQUAL_UINT8(10, b.parts.qualityGate);
    TEST_ASSERT_TRUE(b.score < a.score);
}

void test_score_clamped() {
    RfScoreFeatures f;
    f.ownRssi = -90;
    f.rssiJitter = 40;
    f.densityProxy = 80;
    f.pressureProxy = 80;
    f.occupancyProxy = 100;
    f.stability = 0;
    f.connected = false;
    f.measurementQuality = 10;
    auto r = RfScore::computeV2(f);
    TEST_ASSERT_TRUE(r.score <= 100);
}

void test_v2_softer_than_v1_on_density() {
    auto f = baseFeat();
    f.densityProxy = 48;
    f.pressureProxy = 40;
    auto v1 = RfScore::computeV1(f);
    auto v2 = RfScore::computeV2(f);
    // Same features: v2 density/pressure penalties should be <= v1.
    TEST_ASSERT_TRUE(v2.parts.density <= v1.parts.density);
    TEST_ASSERT_TRUE(v2.parts.pressure <= v1.parts.pressure);
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_v2_lab_like_is_high);
    RUN_TEST(test_v1_lab_like_is_high);
    RUN_TEST(test_rssi_drop_lowers_score);
    RUN_TEST(test_disconnect_penalty);
    RUN_TEST(test_density_pressure_lower_score);
    RUN_TEST(test_occupancy_v2_only);
    RUN_TEST(test_quality_gate);
    RUN_TEST(test_score_clamped);
    RUN_TEST(test_v2_softer_than_v1_on_density);
    return UNITY_END();
}
