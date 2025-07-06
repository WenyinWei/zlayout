#!/usr/bin/env python3
"""
EDA组件系统完整演示
展示如何结合使用数据库存储的基本组件和类定义的复杂逻辑电路
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), "zlayout"))

from zlayout.component_db import ComponentDatabase, ComponentSpec
from zlayout.logic_circuits import FlipFlop, Counter, ProcessorFSM, LogicState

def create_analog_front_end(db: ComponentDatabase):
    """创建模拟前端模块"""
    print("=== 创建模拟前端模块 ===")
    
    # 创建基本模拟组件
    components = []
    
    # 输入滤波器
    input_filter_r = db.create_custom_component(
        name="input_filter_resistor",
        parameters={"resistance": 10000, "tolerance": 0.01},
        electrical_params={"max_voltage": 5.0, "power_rating": 0.1},
        description="输入滤波电阻"
    )
    components.append(input_filter_r)
    
    input_filter_c = db.create_custom_component(
        name="input_filter_capacitor", 
        parameters={"capacitance": 1e-6, "tolerance": 0.05},
        electrical_params={"voltage_rating": 10.0, "esr": 0.01},
        description="输入滤波电容"
    )
    components.append(input_filter_c)
    
    # 放大器
    op_amp = db.create_custom_component(
        name="precision_op_amp",
        parameters={
            "gain": 1000,
            "bandwidth": 10e6,
            "input_offset": 1e-6,
            "noise_density": 10e-9
        },
        electrical_params={
            "supply_voltage": [2.7, 5.5],
            "input_impedance": 1e12,
            "output_impedance": 0.1
        },
        description="精密运算放大器"
    )
    components.append(op_amp)
    
    # 创建模拟前端模块
    afe_module = db.create_module(
        name="analog_front_end",
        components=components,
        connections=[
            {"from": "input", "to": input_filter_r, "pin": "1"},
            {"from": input_filter_r, "pin": "2", "to": input_filter_c, "pin": "1"},
            {"from": input_filter_c, "pin": "2", "to": "gnd"},
            {"from": input_filter_r, "pin": "2", "to": op_amp, "pin": "input+"},
            {"from": op_amp, "pin": "output", "to": "adc_input"}
        ],
        description="模拟前端：包含滤波器和放大器"
    )
    
    print(f"✓ 创建模拟前端模块: {afe_module}")
    print(f"  - 包含 {len(components)} 个组件")
    print(f"  - 输入滤波器 (R={10000}Ω, C={1e-6}F)")
    print(f"  - 精密运算放大器 (增益={1000}, 带宽={10e6}Hz)")
    
    return afe_module

def create_digital_processing_unit():
    """创建数字处理单元"""
    print("\n=== 创建数字处理单元 ===")
    
    # 创建ADC控制状态机
    adc_fsm = ProcessorFSM("ADC_Controller")
    
    # 添加ADC特定状态
    adc_fsm.add_state("SAMPLE")
    adc_fsm.add_state("HOLD")
    adc_fsm.add_state("CONVERT")
    adc_fsm.add_state("READY")
    
    # 添加ADC状态转移
    adc_fsm.add_transition("IDLE", "SAMPLE", "start_conversion")
    adc_fsm.add_transition("SAMPLE", "HOLD", "sample_complete")
    adc_fsm.add_transition("HOLD", "CONVERT", "hold_complete")
    adc_fsm.add_transition("CONVERT", "READY", "conversion_complete")
    adc_fsm.add_transition("READY", "IDLE", "data_read")
    
    # 添加ADC输入信号
    adc_fsm.add_input("start_conversion")
    adc_fsm.add_input("sample_complete")
    adc_fsm.add_input("hold_complete")
    adc_fsm.add_input("conversion_complete")
    adc_fsm.add_input("data_read")
    
    # 创建数据缓冲区 (16位移位寄存器)
    data_buffer = Counter("Data_Buffer", width=16, count_up=True)
    
    # 创建控制逻辑
    control_ff = FlipFlop("Control_FF", "JK")
    
    print("✓ 创建数字处理单元:")
    print(f"  - ADC控制状态机: {len(adc_fsm.states)} 个状态")
    print(f"  - 16位数据缓冲区")
    print(f"  - JK触发器控制逻辑")
    
    return adc_fsm, data_buffer, control_ff

def create_mixed_signal_system(db: ComponentDatabase):
    """创建混合信号系统"""
    print("\n=== 创建混合信号系统 ===")
    
    # 创建模拟前端
    afe_module = create_analog_front_end(db)
    
    # 创建数字处理单元
    adc_fsm, data_buffer, control_ff = create_digital_processing_unit()
    
    # 创建时钟管理
    clock_components = []
    
    # 晶体振荡器
    crystal_osc = db.create_custom_component(
        name="crystal_oscillator",
        parameters={
            "frequency": 100e6,
            "stability": 50e-6,
            "load_capacitance": 12e-12
        },
        electrical_params={
            "supply_voltage": 3.3,
            "current_consumption": 5e-3
        },
        description="100MHz晶体振荡器"
    )
    clock_components.append(crystal_osc)
    
    # 时钟分频器
    clock_divider = Counter("Clock_Divider", width=8, count_up=True)
    
    # PLL
    pll = db.create_custom_component(
        name="phase_locked_loop",
        parameters={
            "input_frequency": 100e6,
            "output_frequency": 400e6,
            "multiplication_factor": 4,
            "jitter": 1e-12
        },
        electrical_params={
            "supply_voltage": 1.8,
            "current_consumption": 20e-3
        },
        description="4倍频PLL"
    )
    clock_components.append(pll)
    
    clock_module = db.create_module(
        name="clock_management",
        components=clock_components,
        connections=[
            {"from": crystal_osc, "to": "pll_input"},
            {"from": pll, "to": "system_clock"},
            {"from": "system_clock", "to": "divider_input"}
        ],
        description="时钟管理模块"
    )
    
    print("✓ 创建混合信号系统:")
    print(f"  - 模拟前端模块: {afe_module}")
    print(f"  - 数字处理单元: ADC状态机 + 数据缓冲区")
    print(f"  - 时钟管理模块: {clock_module}")
    
    return {
        "afe_module": afe_module,
        "adc_fsm": adc_fsm,
        "data_buffer": data_buffer,
        "control_ff": control_ff,
        "clock_module": clock_module,
        "clock_divider": clock_divider
    }

def simulate_system_operation(system_components):
    """模拟系统操作"""
    print("\n=== 系统操作模拟 ===")
    
    adc_fsm = system_components["adc_fsm"]
    data_buffer = system_components["data_buffer"]
    control_ff = system_components["control_ff"]
    clock_divider = system_components["clock_divider"]
    
    # 启用所有组件
    data_buffer.enable_signal.set_state(LogicState.HIGH)
    control_ff.enable_signal.set_state(LogicState.HIGH)
    clock_divider.enable_signal.set_state(LogicState.HIGH)
    
    print("--- 时钟分频器操作 ---")
    for i in range(5):
        clock_divider.on_clock_edge()
        count = clock_divider.internal_state["count"]
        print(f"时钟周期 {i+1}: 分频计数 = {count}")
    
    print("\n--- ADC控制状态机操作 ---")
    # 模拟ADC转换过程
    adc_fsm.input_signals["start_conversion"].set_state(LogicState.HIGH)
    adc_fsm.process_inputs()
    
    adc_fsm.input_signals["sample_complete"].set_state(LogicState.HIGH)
    adc_fsm.process_inputs()
    
    adc_fsm.input_signals["hold_complete"].set_state(LogicState.HIGH)
    adc_fsm.process_inputs()
    
    adc_fsm.input_signals["conversion_complete"].set_state(LogicState.HIGH)
    adc_fsm.process_inputs()
    
    adc_fsm.input_signals["data_read"].set_state(LogicState.HIGH)
    adc_fsm.process_inputs()
    
    print("\n--- 数据缓冲区操作 ---")
    for i in range(3):
        data_buffer.on_clock_edge()
        count = data_buffer.internal_state["count"]
        print(f"数据采样 {i+1}: 缓冲区值 = {count}")
    
    print("\n--- 控制逻辑操作 ---")
    control_ff.inputs["J"].set_state(LogicState.HIGH)
    control_ff.inputs["K"].set_state(LogicState.LOW)
    control_ff.on_clock_edge()
    print(f"JK触发器状态: Q = {control_ff.outputs['Q'].state}")

def analyze_system_complexity(db: ComponentDatabase, system_components):
    """分析系统复杂度"""
    print("\n=== 系统复杂度分析 ===")
    
    # 统计数据库中的组件
    library = db.get_component_library()
    total_db_components = sum(library.values())
    
    # 统计类定义的组件
    logic_components = [
        "ADC状态机",
        "数据缓冲区",
        "控制触发器",
        "时钟分频器"
    ]
    
    print(f"数据库存储组件: {total_db_components} 个")
    for category, count in library.items():
        print(f"  - {category}: {count} 个")
    
    print(f"\n类定义的逻辑组件: {len(logic_components)} 个")
    for comp in logic_components:
        print(f"  - {comp}")
    
    print(f"\n总组件数: {total_db_components + len(logic_components)}")
    
    # 分析优势
    print("\n--- 架构优势分析 ---")
    print("✓ 数据库存储:")
    print("  - 灵活的参数定义")
    print("  - 支持用户自定义组件")
    print("  - 支持中间模块")
    print("  - 易于搜索和管理")
    
    print("\n✓ 类定义:")
    print("  - 复杂的时序逻辑建模")
    print("  - 状态机行为仿真")
    print("  - 实时信号处理")
    print("  - 时序约束检查")
    
    print("\n✓ 混合架构:")
    print("  - 避免了过度的类继承")
    print("  - 提供了最大的灵活性")
    print("  - 适合EDA工具的实际需求")
    print("  - 支持十亿级组件设计")

def main():
    """主函数"""
    print("=== EDA组件系统完整演示 ===")
    print("展示数据库存储 + 类定义的混合架构")
    
    # 创建数据库
    db = ComponentDatabase("mixed_signal_system.db")
    
    # 创建混合信号系统
    system_components = create_mixed_signal_system(db)
    
    # 模拟系统操作
    simulate_system_operation(system_components)
    
    # 分析系统复杂度
    analyze_system_complexity(db, system_components)
    
    print("\n=== 演示完成 ===")
    print("✓ 成功展示了混合架构的优势")
    print("✓ 数据库存储提供了灵活性")
    print("✓ 类定义支持了复杂逻辑建模")
    print("✓ 这种架构更适合实际EDA工具开发")
    
    db.close()

if __name__ == "__main__":
    main() 