#!/usr/bin/env python3
"""
EDA组件接口 - 统一的组件管理接口
提供数据库存储组件和类定义逻辑电路的统一访问接口
"""

from typing import Dict, Any, Optional, Union, List
from abc import ABC, abstractmethod
import json
from enum import Enum

from .component_db import ComponentDatabase, ComponentSpec
from .logic_circuits import (
    FlipFlop, Counter, ProcessorFSM, StateMachine,
    SequentialLogic, Signal, LogicState
)

class ComponentJSONEncoder(json.JSONEncoder):
    """Custom JSON encoder for component objects"""
    
    def default(self, obj):
        if isinstance(obj, LogicState):
            # Convert LogicState to its name (string representation)
            return obj.name
        elif isinstance(obj, Enum):
            # Handle other enums by converting to their value
            return obj.value
        elif hasattr(obj, '__dict__'):
            # For other complex objects, try to serialize their __dict__
            return obj.__dict__
        else:
            # Let the base class default method raise the TypeError
            return super().default(obj)

class ComponentType(Enum):
    """组件类型枚举"""
    DATABASE = "database"      # 数据库存储的组件
    LOGIC_CLASS = "logic_class"  # 类定义的逻辑电路
    MODULE = "module"          # 复合模块

class ComponentCategory(Enum):
    """组件类别枚举"""
    PASSIVE = "passive"        # 无源器件 (R, L, C)
    ACTIVE = "active"          # 有源器件 (运放, 晶体管)
    DIGITAL = "digital"        # 数字逻辑
    ANALOG = "analog"          # 模拟电路
    MIXED_SIGNAL = "mixed_signal"  # 混合信号
    CUSTOM = "custom"          # 用户自定义

class ComponentInterface(ABC):
    """组件接口基类"""
    
    def __init__(self, name: str, category: ComponentCategory):
        self.name = name
        self.category = category
        self.component_type = None
        self.parameters = {}
        self.connections = {}
        
    @abstractmethod
    def get_parameters(self) -> Dict[str, Any]:
        """获取组件参数"""
        pass
    
    @abstractmethod
    def set_parameter(self, name: str, value: Any):
        """设置组件参数"""
        pass
    
    @abstractmethod
    def get_info(self) -> Dict[str, Any]:
        """获取组件信息"""
        pass
    
    @abstractmethod
    def connect(self, pin: str, target, target_pin=None):
        """连接组件"""
        pass

class DatabaseComponent(ComponentInterface):
    """数据库存储组件的包装器"""
    
    def __init__(self, component_spec: ComponentSpec, db: ComponentDatabase):
        super().__init__(component_spec.name, ComponentCategory.CUSTOM)
        self.component_type = ComponentType.DATABASE
        self.spec = component_spec
        self.db = db
        self.parameters = component_spec.parameters.copy()
        
    def get_parameters(self) -> Dict[str, Any]:
        return self.parameters
    
    def set_parameter(self, name: str, value: Any):
        self.parameters[name] = value
        # 更新组件规格
        self.spec.parameters[name] = value
    
    def get_info(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "type": self.component_type.value,
            "category": self.category.value,
            "parameters": self.parameters,
            "electrical_params": self.spec.electrical_params,
            "physical_params": self.spec.physical_params,
            "description": self.spec.description
        }
    
    def connect(self, pin: str, target, target_pin=None):
        if isinstance(target, str):
            self.connections[pin] = {"target": target, "target_pin": target_pin}
        else:
            self.connections[pin] = {"target": target.name, "target_pin": target_pin}

class LogicComponent(ComponentInterface):
    """逻辑电路组件的包装器"""
    
    def __init__(self, logic_circuit):
        super().__init__(logic_circuit.name, ComponentCategory.DIGITAL)
        self.component_type = ComponentType.LOGIC_CLASS
        self.logic_circuit = logic_circuit
        self.parameters = {}
        
        # 安全地提取逻辑电路的参数
        if hasattr(logic_circuit, 'width'):
            self.parameters['width'] = getattr(logic_circuit, 'width')
        if hasattr(logic_circuit, 'ff_type'):
            self.parameters['flip_flop_type'] = getattr(logic_circuit, 'ff_type')
        if hasattr(logic_circuit, 'count_up'):
            self.parameters['count_up'] = getattr(logic_circuit, 'count_up')
    
    def get_parameters(self) -> Dict[str, Any]:
        # 动态获取当前状态
        if hasattr(self.logic_circuit, 'internal_state'):
            self.parameters['current_state'] = self.logic_circuit.internal_state
        if hasattr(self.logic_circuit, 'current_state'):
            self.parameters['fsm_state'] = self.logic_circuit.current_state
        return self.parameters
    
    def set_parameter(self, name: str, value: Any):
        self.parameters[name] = value
        # 某些参数可能需要重新配置逻辑电路
        if name == 'width' and hasattr(self.logic_circuit, 'width'):
            setattr(self.logic_circuit, 'width', value)
        elif name == 'count_up' and hasattr(self.logic_circuit, 'count_up'):
            setattr(self.logic_circuit, 'count_up', value)
    
    def get_info(self) -> Dict[str, Any]:
        info = {
            "name": self.name,
            "type": self.component_type.value,
            "category": self.category.value,
            "parameters": self.get_parameters(),
            "description": f"{self.logic_circuit.__class__.__name__} 逻辑电路"
        }
        
        # 添加逻辑电路特有信息
        if hasattr(self.logic_circuit, 'inputs'):
            info['inputs'] = list(self.logic_circuit.inputs.keys())
        if hasattr(self.logic_circuit, 'outputs'):
            info['outputs'] = list(self.logic_circuit.outputs.keys())
        if hasattr(self.logic_circuit, 'states'):
            info['states'] = list(self.logic_circuit.states)
        
        return info
    
    def connect(self, pin: str, target, target_pin=None):
        if isinstance(target, str):
            self.connections[pin] = {"target": target, "target_pin": target_pin}
        else:
            self.connections[pin] = {"target": target.name, "target_pin": target_pin}
    
    def set_input(self, input_name: str, value: LogicState):
        """设置逻辑输入"""
        if hasattr(self.logic_circuit, 'inputs') and input_name in self.logic_circuit.inputs:
            self.logic_circuit.inputs[input_name].set_state(value)
    
    def get_output(self, output_name: str) -> LogicState:
        """获取逻辑输出"""
        if hasattr(self.logic_circuit, 'outputs') and output_name in self.logic_circuit.outputs:
            return self.logic_circuit.outputs[output_name].state
        return LogicState.UNKNOWN
    
    def simulate_step(self):
        """执行一步仿真"""
        if hasattr(self.logic_circuit, 'on_clock_edge'):
            self.logic_circuit.on_clock_edge()
        elif hasattr(self.logic_circuit, 'process_inputs'):
            self.logic_circuit.process_inputs()

class ComponentFactory:
    """组件工厂类"""
    
    def __init__(self, db: ComponentDatabase):
        self.db = db
        self.custom_types = {}
        
    def create_component(self, component_name: str, component_type: str = None, **kwargs) -> ComponentInterface:
        """创建组件实例"""
        
        # 首先尝试从数据库创建
        try:
            spec = self.db.get_component(component_name)
            if spec:
                return DatabaseComponent(spec, self.db)
        except:
            pass
        
        # 尝试创建逻辑电路
        if component_type:
            logic_circuit = self._create_logic_circuit(component_name, component_type, **kwargs)
            if logic_circuit:
                return LogicComponent(logic_circuit)
        
        # 创建自定义组件
        component_id = self.db.create_custom_component(component_name, **kwargs)
        spec = self.db.get_component(component_id)
        if spec:
            return DatabaseComponent(spec, self.db)
        else:
            raise ValueError(f"Failed to create component {component_name}")
    
    def _create_logic_circuit(self, name: str, circuit_type: str, **kwargs):
        """创建逻辑电路"""
        circuit_type = circuit_type.lower()
        
        if circuit_type == "flipflop":
            ff_type = kwargs.get("flip_flop_type", "D")
            return FlipFlop(name, ff_type)
        elif circuit_type == "counter":
            width = kwargs.get("width", 8)
            count_up = kwargs.get("count_up", True)
            return Counter(name, width, count_up)
        elif circuit_type == "statemachine":
            return StateMachine(name)
        elif circuit_type == "processorfsm":
            return ProcessorFSM(name)
        elif circuit_type in self.custom_types:
            return self.custom_types[circuit_type](name, **kwargs)
        
        return None
    
    def register_custom_type(self, type_name: str, circuit_class):
        """注册自定义逻辑电路类型"""
        self.custom_types[type_name] = circuit_class

class ComponentManager:
    """组件管理器 - 提供统一的组件管理接口"""
    
    def __init__(self, db_path: str = "components.db"):
        self.db = ComponentDatabase(db_path)
        self.factory = ComponentFactory(self.db)
        self.components = {}  # 实例化的组件
        self.modules = {}     # 模块定义
        
    def create_component(self, name: str, component_type: str = None, **kwargs) -> ComponentInterface:
        """创建组件"""
        component = self.factory.create_component(name, component_type, **kwargs)
        self.components[name] = component
        return component
    
    def get_component(self, name: str) -> Optional[ComponentInterface]:
        """获取组件"""
        return self.components.get(name)
    
    def list_components(self) -> List[str]:
        """列出所有组件"""
        return list(self.components.keys())
    
    def create_module(self, name: str, components: List[ComponentInterface], 
                     connections: List[Dict[str, Any]] = None) -> Dict[str, Any]:
        """创建模块"""
        if connections is None:
            connections = []
        module = {
            "name": name,
            "components": {comp.name: comp for comp in components},
            "connections": connections,
            "type": ComponentType.MODULE
        }
        self.modules[name] = module
        return module
    
    def get_module(self, name: str) -> Optional[Dict[str, Any]]:
        """获取模块"""
        return self.modules.get(name)
    
    def connect_components(self, source: str, source_pin: str, target: str, target_pin: str):
        """连接组件"""
        source_comp = self.get_component(source)
        target_comp = self.get_component(target)
        
        if source_comp and target_comp:
            source_comp.connect(source_pin, target_comp, target_pin)
    
    def simulate_system(self, steps: int = 1):
        """系统仿真"""
        for _ in range(steps):
            for component in self.components.values():
                if isinstance(component, LogicComponent):
                    component.simulate_step()
    
    def get_system_info(self) -> Dict[str, Any]:
        """获取系统信息"""
        return {
            "components": {name: comp.get_info() for name, comp in self.components.items()},
            "modules": {name: {
                "name": module["name"],
                "component_count": len(module["components"]),
                "connection_count": len(module["connections"])
            } for name, module in self.modules.items()},
            "database_stats": self.db.get_component_library()
        }
    
    def export_design(self, filename: str):
        """导出设计"""
        design = {
            "components": {name: comp.get_info() for name, comp in self.components.items()},
            "modules": self.modules,
            "connections": {name: comp.connections for name, comp in self.components.items()}
        }
        
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(design, f, indent=2, ensure_ascii=False, cls=ComponentJSONEncoder)
    
    def close(self):
        """关闭数据库连接"""
        self.db.close()

# 便捷函数
def create_component_manager(db_path: str = "components.db") -> ComponentManager:
    """创建组件管理器"""
    return ComponentManager(db_path)

def create_resistor(name: str, resistance: float, tolerance: float = 0.05) -> ComponentInterface:
    """创建电阻器"""
    manager = ComponentManager()
    return manager.create_component(
        name,
        parameters={"resistance": resistance, "tolerance": tolerance},
        electrical_params={"max_voltage": 50.0, "power_rating": 0.25},
        description=f"{resistance}Ω 电阻器"
    )

def create_capacitor(name: str, capacitance: float, voltage_rating: float = 50.0) -> ComponentInterface:
    """创建电容器"""
    manager = ComponentManager()
    return manager.create_component(
        name,
        parameters={"capacitance": capacitance},
        electrical_params={"voltage_rating": voltage_rating},
        description=f"{capacitance}F 电容器"
    )

def create_flipflop(name: str, ff_type: str = "D") -> ComponentInterface:
    """创建触发器"""
    manager = ComponentManager()
    return manager.create_component(name, "flipflop", flip_flop_type=ff_type)

def create_counter(name: str, width: int = 8, count_up: bool = True) -> ComponentInterface:
    """创建计数器"""
    manager = ComponentManager()
    return manager.create_component(name, "counter", width=width, count_up=count_up) 