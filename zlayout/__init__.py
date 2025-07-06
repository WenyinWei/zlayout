"""
ZLayout - Advanced Electronic Design Automation Layout Library

A high-performance 2D/3D layout processing library optimized for EDA applications.
Features quadtree spatial indexing, efficient polygon processing, and advanced
geometric analysis for electronic circuit layout optimization.
"""

__version__ = "0.1.0"
__author__ = "ZLayout Team"

from .geometry import Point, Rectangle, Polygon
from .spatial import QuadTree, SpatialIndex
from .analysis import PolygonAnalyzer, GeometryProcessor

# Import visualization only if matplotlib is available
try:
    from .visualization import LayoutVisualizer
    _has_visualization = True
except ImportError:
    _has_visualization = False
    LayoutVisualizer = None

__all__ = [
    'Point', 'Rectangle', 'Polygon',
    'QuadTree', 'SpatialIndex', 
    'PolygonAnalyzer', 'GeometryProcessor'
]

if _has_visualization:
    __all__.append('LayoutVisualizer')