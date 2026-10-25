#!/usr/bin/env python3
"""Pure-Python port of src/rf/RfScore.cpp — keep in sync with firmware."""

from __future__ import annotations

from dataclasses import dataclass, asdict
from typing import Any, Dict, Optional


def _clamp_u8(v: float | int) -> int:
    v = int(v)
    if v < 0:
        return 0
    if v > 100:
        return 100
    return v


@dataclass
class Features:
    own_rssi: int = 0
    rssi_jitter: float = 0.0
    density_proxy: int = 0
    pressure_proxy: int = 0
    occupancy_proxy: int = 0
    stability: int = 100
    connected: bool = True
    measurement_quality: int = 100


@dataclass
class Breakdown:
    own_link: int = 0
    jitter: int = 0
    density: int = 0
    pressure: int = 0
    occupancy: int = 0
    stability: int = 0
    disconnect: int = 0
    quality_gate: int = 0


@dataclass
class Result:
    score: int
    version: int
    parts: Breakdown


def compute_v1(f: Features) -> Result:
    p = Breakdown()
    p.own_link = _clamp_u8((-55 - f.own_rssi) * 1.2) if f.own_rssi < -55 else 0
    p.jitter = _clamp_u8((f.rssi_jitter - 8) * 2) if f.rssi_jitter > 8 else 0
    p.pressure = f.pressure_proxy // 4
    p.density = f.density_proxy // 6
    p.occupancy = 0
    p.stability = (100 - f.stability) // 3
    p.disconnect = 0 if f.connected else 25
    p.quality_gate = 0
    score = 100 - (
        p.own_link + p.jitter + p.pressure + p.density + p.stability + p.disconnect
    )
    return Result(score=_clamp_u8(score), version=1, parts=p)


def compute_v2(f: Features) -> Result:
    p = Breakdown()
    p.own_link = _clamp_u8((-50 - f.own_rssi) * 1.0) if f.own_rssi < -50 else 0
    p.jitter = _clamp_u8((f.rssi_jitter - 10) * 1.5) if f.rssi_jitter > 10 else 0
    p.density = f.density_proxy // 8
    p.pressure = f.pressure_proxy // 5
    p.occupancy = f.occupancy_proxy // 20
    p.stability = (100 - f.stability) // 4
    p.disconnect = 0 if f.connected else 20
    if f.measurement_quality < 40:
        p.quality_gate = 10
    elif f.measurement_quality < 60:
        p.quality_gate = 5
    else:
        p.quality_gate = 0
    score = 100 - (
        p.own_link
        + p.jitter
        + p.density
        + p.pressure
        + p.occupancy
        + p.stability
        + p.disconnect
        + p.quality_gate
    )
    return Result(score=_clamp_u8(score), version=2, parts=p)


def features_from_session_sample(s: Dict[str, Any]) -> Features:
    """Best-effort feature rebuild from MeasSession samples."""
    ap = int(s.get("apCount") or 0)
    density = int(s.get("density")) if s.get("density") is not None else min(80, ap * 4)
    pressure = int(s.get("pressure") or 0)
    occupancy = int(s.get("occupancy") or 0)
    jitter = float(s.get("jitter") or 0)
    stability = int(s.get("stability") if s.get("stability") is not None else 100)
    connected = bool(s.get("connected", True))
    quality = int(s.get("quality") if s.get("quality") is not None else 100)
    return Features(
        own_rssi=int(s.get("ownRssi") or 0),
        rssi_jitter=jitter,
        density_proxy=density,
        pressure_proxy=pressure,
        occupancy_proxy=occupancy,
        stability=stability,
        connected=connected,
        measurement_quality=quality,
    )


def features_from_phase_aggregate(ph: Dict[str, Any]) -> Optional[Features]:
    """Rebuild Features from MeasSession per-phase running aggregates (survive ring wrap)."""
    n = int(ph.get("n") or 0)
    if n <= 0:
        return None
    connected_pct = float(ph.get("connectedPct") if ph.get("connectedPct") is not None else 100)
    return Features(
        own_rssi=int(round(float(ph.get("avgOwnRssi") or 0))),
        rssi_jitter=float(ph.get("avgJitter") or 0),
        density_proxy=int(round(float(ph.get("avgDensity") or 0))),
        pressure_proxy=int(round(float(ph.get("avgPressure") or 0))),
        occupancy_proxy=int(round(float(ph.get("avgOccupancy") or 0))),
        stability=int(round(float(ph.get("avgStability") if ph.get("avgStability") is not None else 100))),
        connected=connected_pct >= 50.0,
        measurement_quality=int(
            round(float(ph.get("avgQuality") if ph.get("avgQuality") is not None else 100))
        ),
    )


def result_dict(r: Result) -> Dict[str, Any]:
    return {
        "score": r.score,
        "version": r.version,
        "parts": {
            "ownLink": r.parts.own_link,
            "jitter": r.parts.jitter,
            "density": r.parts.density,
            "pressure": r.parts.pressure,
            "occupancy": r.parts.occupancy,
            "stability": r.parts.stability,
            "disconnect": r.parts.disconnect,
            "qualityGate": r.parts.quality_gate,
        },
    }
