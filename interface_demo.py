#!/usr/bin/env python3
"""
灵活组件接口演示
展示如何使用统一的ComponentManager来管理数据库组件和逻辑电路
"""

import sys
import os
sys.path.append(os.path.dirname(__file__))

from zlayout import (
    ComponentManager, ComponentType, ComponentCategory, LogicState,
    create_component_manager, create_resistor, create_capacitor, 
    create_flipflop, create_counter
)

def demo_basic_component_operations():
    """演示基本组件操作"""
    print("=== 基本组件操作演示 ===")
    
    # 创建组件管理器
    manager = ComponentManager("demo_interface.db")
    
    # 创建各种类型的组件
    print("\n--- 创建组件 ---")
    
    # 数据库存储的模拟组件
    resistor = manager.create_component(
        "R1", 
        parameters={"resistance": 10000, "tolerance": 0.01},
        electrical_params={"max_voltage": 50.0, "power_rating": 0.25},
        description="10kΩ 精密电阻"
    )
    
    capacitor = manager.create_component(
        "C1",
        parameters={"capacitance": 100e-6, "tolerance": 0.05},
        electrical_params={"voltage_rating": 25.0, "esr": 0.01},
        description="100μF 电解电容"
    )
    
    # 类定义的数字逻辑组件
    flipflop = manager.create_component("FF1", "flipflop", flip_flop_type="D")
    counter = manager.create_component("CNT1", "counter", width=8, count_up=True)
    
    print(f"✓ 创建了 {len(manager.list_components())} 个组件")
    for comp_name in manager.list_components():
        comp = manager.get_component(comp_name)
        print(f"  - {comp_name}: {comp.component_type.value} ({comp.category.value})")
    
    return manager, resistor, capacitor, flipflop, counter

def demo_component_information():
    """演示组件信息获取"""
    print("\n=== 组件信息获取演示 ===")
    
    manager, resistor, capacitor, flipflop, counter = demo_basic_component_operations()
    
    print("\n--- 电阻器信息 ---")
    r_info = resistor.get_info()
    print(f"名称: {r_info['name']}")
    print(f"类型: {r_info['type']}")
    print(f"参数: {r_info['parameters']}")
    print(f"电气参数: {r_info['electrical_params']}")
    
    print("\n--- 触发器信息 ---")
    ff_info = flipflop.get_info()
    print(f"名称: {ff_info['name']}")
    print(f"类型: {ff_info['type']}")
    print(f"参数: {ff_info['parameters']}")
    if 'inputs' in ff_info:
        print(f"输入: {ff_info['inputs']}")
    if 'outputs' in ff_info:
        print(f"输出: {ff_info['outputs']}")
    
    manager.close()

def demo_parameter_modification():
    """演示参数修改"""
    print("\n=== 参数修改演示 ===")
    
    manager = ComponentManager("demo_interface.db")
    
    # 创建一个计数器
    counter = manager.create_component("CNT_DEMO", "counter", width=4, count_up=True)
    
    print("--- 修改前 ---")
    print(f"计数器宽度: {counter.get_parameters().get('width', 'N/A')}")
    print(f"计数方向: {counter.get_parameters().get('count_up', 'N/A')}")
    
    # 修改参数
    counter.set_parameter("width", 8)
    counter.set_parameter("count_up", False)
    
    print("\n--- 修改后 ---")
    print(f"计数器宽度: {counter.get_parameters().get('width', 'N/A')}")
    print(f"计数方向: {counter.get_parameters().get('count_up', 'N/A')}")
    
    manager.close()

def demo_component_connections():
    """演示组件连接"""
    print("\n=== 组件连接演示 ===")
    
    manager = ComponentManager("demo_interface.db")
    
    # 创建组件
    resistor = manager.create_component(
        "R_CONN", 
        parameters={"resistance": 1000},
        description="连接演示电阻"
    )
    
    capacitor = manager.create_component(
        "C_CONN",
        parameters={"capacitance": 10e-6},
        description="连接演示电容"
    )
    
    flipflop = manager.create_component("FF_CONN", "flipflop", flip_flop_type="D")
    
    # 建立连接
    resistor.connect("pin2", capacitor, "pin1")
    capacitor.connect("pin2", flipflop, "D")
    
    print("--- 连接信息 ---")
    print(f"电阻连接: {resistor.connections}")
    print(f"电容连接: {capacitor.connections}")
    print(f"触发器连接: {flipflop.connections}")
    
    # 也可以使用管理器来连接
    manager.connect_components("R_CONN", "pin1", "C_CONN", "pin2")
    
    manager.close()

def demo_logic_simulation():
    """演示逻辑仿真"""
    print("\n=== 逻辑仿真演示 ===")
    
    manager = ComponentManager("demo_interface.db")
    
    # 创建逻辑组件
    flipflop = manager.create_component("FF_SIM", "flipflop", flip_flop_type="D")
    counter = manager.create_component("CNT_SIM", "counter", width=4, count_up=True)
    
    print("--- 触发器仿真 ---")
    ff_logic = flipflop
    if hasattr(ff_logic, 'set_input'):
        ff_logic.set_input("D", LogicState.HIGH)
        print(f"设置 D 输入为 HIGH")
        
        # 模拟时钟边沿
        ff_logic.simulate_step()
        
        if hasattr(ff_logic, 'get_output'):
            q_output = ff_logic.get_output("Q")
            print(f"Q 输出: {q_output}")
    
    print("\n--- 计数器仿真 ---")
    counter_logic = counter
    print("计数器状态变化:")
    for i in range(3):
        counter_logic.simulate_step()
        params = counter_logic.get_parameters()
        current_state = params.get('current_state', {})
        count = current_state.get('count', 0)
        print(f"  步骤 {i+1}: 计数值 = {count}")
    
    manager.close()

def demo_module_creation():
    """演示模块创建"""
    print("\n=== 模块创建演示 ===")
    
    manager = ComponentManager("demo_interface.db")
    
    # 创建模块组件
    components = []
    
    # 滤波器组件
    filter_r = manager.create_component(
        "FILTER_R",
        parameters={"resistance": 10000},
        description="滤波器电阻"
    )
    components.append(filter_r)
    
    filter_c = manager.create_component(
        "FILTER_C",
        parameters={"capacitance": 100e-9},
        description="滤波器电容"
    )
    components.append(filter_c)
    
    # 数字处理组件
    adc_ff = manager.create_component("ADC_FF", "flipflop", flip_flop_type="D")
    components.append(adc_ff)
    
    # 创建模块
    connections = [
        {"from": "input", "to": "FILTER_R", "pin": "pin1"},
        {"from": "FILTER_R", "pin": "pin2", "to": "FILTER_C", "pin": "pin1"},
        {"from": "FILTER_C", "pin": "pin2", "to": "ADC_FF", "pin": "D"},
        {"from": "ADC_FF", "pin": "Q", "to": "output"}
    ]
    
    filter_module = manager.create_module(
        "RC_FILTER_MODULE",
        components,
        connections
    )
    
    print(f"✓ 创建模块: {filter_module['name']}")
    print(f"  - 组件数量: {len(filter_module['components'])}")
    print(f"  - 连接数量: {len(filter_module['connections'])}")
    
    # 获取模块信息
    module_info = manager.get_module("RC_FILTER_MODULE")
    if module_info:
        print(f"  - 模块类型: {module_info['type'].value}")
        print(f"  - 包含组件: {list(module_info['components'].keys())}")
    
    manager.close()

def demo_system_analysis():
    """演示系统分析"""
    print("\n=== 系统分析演示 ===")
    
    manager = ComponentManager("demo_interface.db")
    
    # 创建混合系统
    # 模拟前端
    manager.create_component("INPUT_R", parameters={"resistance": 50})
    manager.create_component("INPUT_C", parameters={"capacitance": 10e-12})
    
    # 数字后端
    manager.create_component("DATA_FF", "flipflop", flip_flop_type="JK")
    manager.create_component("ADDR_CNT", "counter", width=16, count_up=True)
    
    # 获取系统信息
    system_info = manager.get_system_info()
    
    print("--- 系统组件统计 ---")
    print(f"总组件数: {len(system_info['components'])}")
    
    # 按类型分类
    type_counts = {}
    for comp_info in system_info['components'].values():
        comp_type = comp_info['type']
        type_counts[comp_type] = type_counts.get(comp_type, 0) + 1
    
    for comp_type, count in type_counts.items():
        print(f"  - {comp_type}: {count} 个")
    
    print(f"\n--- 数据库统计 ---")
    db_stats = system_info['database_stats']
    total_db_components = sum(db_stats.values())
    print(f"数据库中的组件类型: {total_db_components} 个")
    
    # 导出设计
    manager.export_design("demo_design.json")
    print("\n✓ 设计已导出到 demo_design.json")
    
    manager.close()

def demo_convenience_functions():
    """演示便捷函数"""
    print("\n=== 便捷函数演示 ===")
    
    # 使用便捷函数创建组件
    resistor = create_resistor("CONV_R", 2200, 0.01)
    capacitor = create_capacitor("CONV_C", 47e-6, 16.0)
    flipflop = create_flipflop("CONV_FF", "JK")
    counter = create_counter("CONV_CNT", 12, False)
    
    print("--- 便捷函数创建的组件 ---")
    components = [resistor, capacitor, flipflop, counter]
    
    for comp in components:
        info = comp.get_info()
        print(f"  - {info['name']}: {info['type']} ({info.get('description', 'N/A')})")

def main():
    """主函数"""
    print("=== 灵活组件接口系统演示 ===")
    print("展示统一的ComponentManager管理数据库组件和逻辑电路")
    
    demo_component_information()
    demo_parameter_modification()
    demo_component_connections()
    demo_logic_simulation()
    demo_module_creation()
    demo_system_analysis()
    demo_convenience_functions()
    
    print("\n=== 演示完成 ===")
    print("✓ 统一接口简化了组件管理")
    print("✓ 数据库组件提供了灵活的参数存储")
    print("✓ 逻辑电路支持复杂的时序仿真")
    print("✓ 模块化设计支持层次化系统")
    print("✓ 便捷函数提供了快速创建组件的方法")
    
    # 清理演示文件
    cleanup_files = ["demo_interface.db", "demo_design.json"]
    for file in cleanup_files:
        if os.path.exists(file):
            os.remove(file)
            print(f"✓ 清理文件: {file}")

if __name__ == "__main__":
    main() 