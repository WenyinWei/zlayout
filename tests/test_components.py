#!/usr/bin/env python3
"""
Unit tests for zlayout component system.
"""

import unittest
import tempfile
import os
import sys

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from zlayout.component_db import ComponentDatabase, ComponentSpec
from zlayout.logic_circuits import FlipFlop, Counter, ProcessorFSM, LogicState
from zlayout.component_interface import (
    ComponentManager, ComponentType, ComponentCategory,
    DatabaseComponent, LogicComponent, ComponentFactory
)


class TestComponentDatabase(unittest.TestCase):
    """Test cases for ComponentDatabase."""
    
    def setUp(self):
        # Create temporary database file
        self.temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        self.temp_db.close()
        self.db = ComponentDatabase(self.temp_db.name)
    
    def tearDown(self):
        self.db.close()
        if os.path.exists(self.temp_db.name):
            os.unlink(self.temp_db.name)
    
    def test_database_initialization(self):
        """Test database initialization and basic components."""
        library = self.db.get_component_library()
        self.assertGreater(len(library), 0)
        self.assertIn('passive', library)
    
    def test_custom_component_creation(self):
        """Test creating custom components."""
        component_id = self.db.create_custom_component(
            name="test_resistor",
            parameters={"resistance": 1000, "tolerance": 0.05},
            electrical_params={"max_voltage": 50.0},
            description="Test resistor"
        )
        
        self.assertIsInstance(component_id, str)
        self.assertGreater(len(component_id), 0)
        
        # Retrieve the component
        spec = self.db.get_component(component_id)
        self.assertIsNotNone(spec)
        if spec:
            self.assertEqual(spec.name, "test_resistor")
            self.assertEqual(spec.parameters["resistance"], 1000)
    
    def test_component_search(self):
        """Test component search functionality."""
        # Create a test component
        self.db.create_custom_component(
            name="search_test_capacitor",
            parameters={"capacitance": 100e-6},
            description="Search test capacitor"
        )
        
        # Search by name pattern
        results = self.db.search_components(name_pattern="search_test")
        self.assertGreater(len(results), 0)
        
        # Search by category
        results = self.db.search_components(category="custom")
        self.assertGreater(len(results), 0)
    
    def test_module_creation(self):
        """Test module creation and retrieval."""
        # Create some components first
        comp1_id = self.db.create_custom_component("mod_comp1", {"value": 1})
        comp2_id = self.db.create_custom_component("mod_comp2", {"value": 2})
        
        # Create module
        module_id = self.db.create_module(
            name="test_module",
            components=[comp1_id, comp2_id],
            connections=[{"from": comp1_id, "to": comp2_id}],
            description="Test module"
        )
        
        self.assertIsInstance(module_id, str)
        self.assertGreater(len(module_id), 0)


class TestLogicCircuits(unittest.TestCase):
    """Test cases for logic circuits."""
    
    def test_flipflop_creation(self):
        """Test FlipFlop creation and basic operations."""
        ff = FlipFlop("test_ff", "D")
        self.assertEqual(ff.name, "test_ff")
        self.assertEqual(ff.ff_type, "D")
        self.assertIn("D", ff.inputs)
        self.assertIn("Q", ff.outputs)
    
    def test_flipflop_simulation(self):
        """Test FlipFlop simulation."""
        ff = FlipFlop("sim_ff", "D")
        ff.enable_signal.set_state(LogicState.HIGH)
        
        # Set D input
        ff.inputs["D"].set_state(LogicState.HIGH)
        ff.on_clock_edge()
        
        # Check Q output
        self.assertEqual(ff.outputs["Q"].state, LogicState.HIGH)
        self.assertEqual(ff.outputs["Q_bar"].state, LogicState.LOW)
    
    def test_counter_creation(self):
        """Test Counter creation and basic operations."""
        counter = Counter("test_counter", width=4, count_up=True)
        self.assertEqual(counter.name, "test_counter")
        self.assertEqual(counter.width, 4)
        self.assertTrue(counter.count_up)
    
    def test_counter_simulation(self):
        """Test Counter simulation."""
        counter = Counter("sim_counter", width=4, count_up=True)
        counter.enable_signal.set_state(LogicState.HIGH)
        
        # Initial count should be 0
        self.assertEqual(counter.internal_state["count"], 0)
        
        # Count up
        counter.on_clock_edge()
        self.assertEqual(counter.internal_state["count"], 1)
        
        counter.on_clock_edge()
        self.assertEqual(counter.internal_state["count"], 2)
    
    def test_processor_fsm(self):
        """Test ProcessorFSM state machine."""
        fsm = ProcessorFSM("test_cpu")
        
        # Check initial state
        self.assertEqual(fsm.current_state, "IDLE")
        
        # Test state transition
        fsm.input_signals["start"].set_state(LogicState.HIGH)
        fsm.process_inputs()
        self.assertEqual(fsm.current_state, "FETCH")


class TestComponentInterface(unittest.TestCase):
    """Test cases for component interface system."""
    
    def setUp(self):
        # Create temporary database
        self.temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        self.temp_db.close()
        self.manager = ComponentManager(self.temp_db.name)
    
    def tearDown(self):
        self.manager.close()
        if os.path.exists(self.temp_db.name):
            os.unlink(self.temp_db.name)
    
    def test_component_manager_creation(self):
        """Test ComponentManager initialization."""
        self.assertIsInstance(self.manager, ComponentManager)
        self.assertIsInstance(self.manager.db, ComponentDatabase)
        self.assertIsInstance(self.manager.factory, ComponentFactory)
    
    def test_database_component_creation(self):
        """Test creating database components through manager."""
        resistor = self.manager.create_component(
            "test_resistor",
            parameters={"resistance": 1000},
            description="Test resistor"
        )
        
        self.assertIsInstance(resistor, DatabaseComponent)
        self.assertEqual(resistor.component_type, ComponentType.DATABASE)
        self.assertEqual(resistor.name, "test_resistor")
        self.assertEqual(resistor.parameters["resistance"], 1000)
    
    def test_logic_component_creation(self):
        """Test creating logic components through manager."""
        flipflop = self.manager.create_component(
            "test_ff", 
            "flipflop", 
            flip_flop_type="D"
        )
        
        self.assertIsInstance(flipflop, LogicComponent)
        self.assertEqual(flipflop.component_type, ComponentType.LOGIC_CLASS)
        self.assertEqual(flipflop.name, "test_ff")
    
    def test_component_parameters(self):
        """Test component parameter manipulation."""
        counter = self.manager.create_component(
            "test_counter",
            "counter",
            width=8,
            count_up=True
        )
        
        # Get parameters
        params = counter.get_parameters()
        self.assertEqual(params["width"], 8)
        self.assertTrue(params["count_up"])
        
        # Set parameters
        counter.set_parameter("width", 16)
        updated_params = counter.get_parameters()
        self.assertEqual(updated_params["width"], 16)
    
    def test_component_connections(self):
        """Test component connection functionality."""
        # Create components
        resistor = self.manager.create_component(
            "conn_resistor",
            parameters={"resistance": 1000}
        )
        
        capacitor = self.manager.create_component(
            "conn_capacitor", 
            parameters={"capacitance": 100e-6}
        )
        
        # Test connection
        resistor.connect("pin2", capacitor, "pin1")
        
        # Check connection
        self.assertIn("pin2", resistor.connections)
        self.assertEqual(resistor.connections["pin2"]["target"], "conn_capacitor")
        self.assertEqual(resistor.connections["pin2"]["target_pin"], "pin1")
    
    def test_module_creation(self):
        """Test module creation through manager."""
        # Create components
        components = []
        for i in range(3):
            comp = self.manager.create_component(
                f"module_comp_{i}",
                parameters={"value": i}
            )
            components.append(comp)
        
        # Create module
        module = self.manager.create_module(
            "test_module",
            components,
            connections=[{"from": "comp0", "to": "comp1"}]
        )
        
        self.assertEqual(module["name"], "test_module")
        self.assertEqual(len(module["components"]), 3)
        self.assertEqual(module["type"], ComponentType.MODULE)
    
    def test_system_simulation(self):
        """Test system-level simulation."""
        # Create a simple logic system
        ff = self.manager.create_component("sim_ff", "flipflop", flip_flop_type="D")
        counter = self.manager.create_component("sim_counter", "counter", width=4)
        
        # Set up logic components for simulation
        if isinstance(ff, LogicComponent) and hasattr(ff, 'logic_circuit'):
            ff.logic_circuit.enable_signal.set_state(LogicState.HIGH)
            ff.logic_circuit.inputs["D"].set_state(LogicState.HIGH)
        
        if isinstance(counter, LogicComponent) and hasattr(counter, 'logic_circuit'):
            counter.logic_circuit.enable_signal.set_state(LogicState.HIGH)
        
        # Run simulation
        self.manager.simulate_system(steps=2)
        
        # Check that simulation ran without errors
        self.assertTrue(True)  # If we get here, simulation didn't crash
    
    def test_system_info(self):
        """Test system information gathering."""
        # Create mixed components
        self.manager.create_component("info_resistor", parameters={"resistance": 1000})
        self.manager.create_component("info_ff", "flipflop", flip_flop_type="JK")
        
        info = self.manager.get_system_info()
        
        self.assertIn("components", info)
        self.assertIn("database_stats", info)
        self.assertEqual(len(info["components"]), 2)
    
    def test_design_export(self):
        """Test design export functionality."""
        # Create a simple design
        self.manager.create_component("export_comp", parameters={"value": 42})
        
        # Export to temporary file
        temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.json')
        temp_file.close()
        
        try:
            self.manager.export_design(temp_file.name)
            
            # Check that file was created and has content
            self.assertTrue(os.path.exists(temp_file.name))
            self.assertGreater(os.path.getsize(temp_file.name), 0)
        finally:
            if os.path.exists(temp_file.name):
                os.unlink(temp_file.name)


class TestConvenienceFunctions(unittest.TestCase):
    """Test cases for convenience functions."""
    
    def test_component_creation_functions(self):
        """Test convenience functions for component creation."""
        from zlayout.component_interface import (
            create_resistor, create_capacitor, create_flipflop, create_counter
        )
        
        # Test resistor creation
        resistor = create_resistor("conv_r", 2200, 0.01)
        self.assertEqual(resistor.name, "conv_r")
        
        # Test capacitor creation  
        capacitor = create_capacitor("conv_c", 47e-6, 16.0)
        self.assertEqual(capacitor.name, "conv_c")
        
        # Test flipflop creation
        flipflop = create_flipflop("conv_ff", "JK")
        self.assertEqual(flipflop.name, "conv_ff")
        
        # Test counter creation
        counter = create_counter("conv_cnt", 12, False)
        self.assertEqual(counter.name, "conv_cnt")


if __name__ == '__main__':
    unittest.main() 