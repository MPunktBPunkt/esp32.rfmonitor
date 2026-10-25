#!/usr/bin/env python3
"""Host unit tests mirroring firmware RfScore (no pytest required)."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from rf_score import Features, compute_v1, compute_v2, features_from_phase_aggregate


def base() -> Features:
    return Features(
        own_rssi=-48,
        rssi_jitter=2,
        density_proxy=12,
        pressure_proxy=10,
        occupancy_proxy=24,
        stability=100,
        connected=True,
        measurement_quality=100,
    )


class TestRfScore(unittest.TestCase):
    def test_lab_like_high(self):
        self.assertGreaterEqual(compute_v2(base()).score, 90)
        self.assertGreaterEqual(compute_v1(base()).score, 90)

    def test_rssi_drop(self):
        g, b = base(), base()
        b.own_rssi = -70
        self.assertLess(compute_v2(b).score, compute_v2(g).score)

    def test_disconnect(self):
        g, b = base(), base()
        b.connected = False
        self.assertEqual(compute_v2(b).parts.disconnect, 20)
        self.assertEqual(compute_v1(b).parts.disconnect, 25)
        self.assertLess(compute_v2(b).score, compute_v2(g).score)

    def test_density_pressure(self):
        g, b = base(), base()
        b.density_proxy = 80
        b.pressure_proxy = 80
        self.assertLess(compute_v2(b).score, compute_v2(g).score)

    def test_occupancy_v2_only(self):
        f = base()
        f.occupancy_proxy = 100
        self.assertEqual(compute_v1(f).parts.occupancy, 0)
        self.assertGreater(compute_v2(f).parts.occupancy, 0)

    def test_quality_gate(self):
        f = base()
        f.measurement_quality = 30
        self.assertEqual(compute_v2(f).parts.quality_gate, 10)

    def test_v2_softer_density(self):
        f = base()
        f.density_proxy = 48
        f.pressure_proxy = 40
        v1, v2 = compute_v1(f), compute_v2(f)
        self.assertLessEqual(v2.parts.density, v1.parts.density)
        self.assertLessEqual(v2.parts.pressure, v1.parts.pressure)

    def test_phase_aggregate_features(self):
        ph = {
            "n": 12,
            "avgOwnRssi": -68,
            "avgJitter": 3.5,
            "avgDensity": 40,
            "avgPressure": 50,
            "avgOccupancy": 60,
            "avgStability": 90,
            "avgQuality": 95,
            "connectedPct": 100,
            "avgScore": 70,
        }
        feat = features_from_phase_aggregate(ph)
        assert feat is not None
        self.assertEqual(feat.own_rssi, -68)
        self.assertEqual(feat.density_proxy, 40)
        self.assertTrue(feat.connected)
        self.assertIsNone(features_from_phase_aggregate({"n": 0}))


if __name__ == "__main__":
    raise SystemExit(unittest.main())
