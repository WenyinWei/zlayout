"""
ZLayout - Advanced Electronic Design Automation Layout Library

A high-performance 2D/3D layout processing library optimized for EDA applications.
Features quadtree spatial indexing, efficient polygon processing, and advanced
geometric analysis for electronic circuit layout optimization.

Now includes a flexible component system with database-stored components and
class-based logic circuits for comprehensive EDA design workflows.
"""

__version__ = "0.1.0"
__author__ = "ZLayout Team"

# Core geometry and spatial analysis
from .geometry import Point, Rectangle, Polygon
from .spatial import QuadTree, SpatialIndex
from .analysis import PolygonAnalyzer, GeometryProcessor

# Component system
from .component_db import ComponentDatabase, ComponentSpec
from .logic_circuits import (
    FlipFlop, Counter, ProcessorFSM, StateMachine, 
    SequentialLogic, Signal, LogicState
)
from .component_interface import (
    ComponentManager, ComponentInterface, ComponentType, ComponentCategory,
    DatabaseComponent, LogicComponent, ComponentFactory,
    create_component_manager, create_resistor, create_capacitor,
    create_flipflop, create_counter
)

# Import visualization only if matplotlib is available
try:
    from .visualization import LayoutVisualizer
    _has_visualization = True
except ImportError:
    _has_visualization = False
    LayoutVisualizer = None

__all__ = [
    # Core geometry and spatial
    'Point', 'Rectangle', 'Polygon',
    'QuadTree', 'SpatialIndex', 
    'PolygonAnalyzer', 'GeometryProcessor',
    
    # Component system
    'ComponentDatabase', 'ComponentSpec',
    'FlipFlop', 'Counter', 'ProcessorFSM', 'StateMachine',
    'SequentialLogic', 'Signal', 'LogicState',
    'ComponentManager', 'ComponentInterface', 'ComponentType', 'ComponentCategory',
    'DatabaseComponent', 'LogicComponent', 'ComponentFactory',
    'create_component_manager', 'create_resistor', 'create_capacitor',
    'create_flipflop', 'create_counter'
]

if _has_visualization:
    __all__.append('LayoutVisualizer')