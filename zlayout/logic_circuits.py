#!/usr/bin/env python3
"""
复杂逻辑电路类系统
只保留有复杂时序逻辑和状态机的组件，其他基本组件使用数据库存储
"""

from typing import Dict, List, Any, Optional, Set, Tuple
from enum import Enum
from dataclasses import dataclass
import time

class LogicState(Enum):
    """逻辑状态"""
    LOW = 0
    HIGH = 1
    UNKNOWN = 2
    HIGH_IMPEDANCE = 3

class ClockEdge(Enum):
    """时钟边沿"""
    RISING = "rising"
    FALLING = "falling"
    BOTH = "both"

@dataclass
class Signal:
    """信号定义"""
    name: str
    state: LogicState = LogicState.UNKNOWN
    timestamp: float = 0.0
    
    def set_state(self, state: LogicState):
        """设置信号状态"""
        self.state = state
        self.timestamp = time.time()

class StateMachine:
    """状态机基类"""
    
    def __init__(self, name: str):
        self.name = name
        self.current_state = "IDLE"
        self.states: Set[str] = {"IDLE"}
        self.transitions: Dict[str, Dict[str, str]] = {}
        self.state_history: List[Tuple[str, float]] = []
        self.input_signals: Dict[str, Signal] = {}
        self.output_signals: Dict[str, Signal] = {}
    
    def add_state(self, state: str):
        """添加状态"""
        self.states.add(state)
    
    def add_transition(self, from_state: str, to_state: str, condition: str):
        """添加状态转移"""
        if from_state not in self.transitions:
            self.transitions[from_state] = {}
        self.transitions[from_state][condition] = to_state
    
    def add_input(self, signal_name: str):
        """添加输入信号"""
        self.input_signals[signal_name] = Signal(signal_name)
    
    def add_output(self, signal_name: str):
        """添加输出信号"""
        self.output_signals[signal_name] = Signal(signal_name)
    
    def process_inputs(self):
        """处理输入信号，子类需要实现"""
        pass
    
    def transition_to(self, new_state: str):
        """状态转移"""
        if new_state in self.states:
            old_state = self.current_state
            self.current_state = new_state
            self.state_history.append((new_state, time.time()))
            print(f"[{self.name}] State transition: {old_state} -> {new_state}")
    
    def get_state_history(self) -> List[Tuple[str, float]]:
        """获取状态历史"""
        return self.state_history

class SequentialLogic:
    """时序逻辑电路基类"""
    
    def __init__(self, name: str):
        self.name = name
        self.clock_signal = Signal("clock")
        self.reset_signal = Signal("reset")
        self.enable_signal = Signal("enable")
        self.inputs: Dict[str, Signal] = {}
        self.outputs: Dict[str, Signal] = {}
        self.internal_state: Dict[str, Any] = {}
        self.clock_edge = ClockEdge.RISING
        self.setup_time = 0.1  # ns
        self.hold_time = 0.1   # ns
        self.propagation_delay = 0.5  # ns
    
    def add_input(self, signal_name: str):
        """添加输入信号"""
        self.inputs[signal_name] = Signal(signal_name)
    
    def add_output(self, signal_name: str):
        """添加输出信号"""
        self.outputs[signal_name] = Signal(signal_name)
    
    def on_clock_edge(self):
        """时钟边沿触发，子类需要实现"""
        pass
    
    def reset(self):
        """复位操作，子类需要实现"""
        pass
    
    def check_timing(self) -> bool:
        """检查时序约束"""
        # 简化的时序检查
        return True

class FlipFlop(SequentialLogic):
    """触发器"""
    
    def __init__(self, name: str, ff_type: str = "D"):
        super().__init__(name)
        self.ff_type = ff_type
        
        if ff_type == "D":
            self.add_input("D")
            self.add_output("Q")
            self.add_output("Q_bar")
        elif ff_type == "JK":
            self.add_input("J")
            self.add_input("K")
            self.add_output("Q")
            self.add_output("Q_bar")
        elif ff_type == "SR":
            self.add_input("S")
            self.add_input("R")
            self.add_output("Q")
            self.add_output("Q_bar")
        
        self.internal_state["Q"] = LogicState.LOW
    
    def on_clock_edge(self):
        """时钟边沿触发"""
        if self.reset_signal.state == LogicState.HIGH:
            self.reset()
            return
        
        if self.enable_signal.state == LogicState.LOW:
            return
        
        if self.ff_type == "D":
            self._handle_d_ff()
        elif self.ff_type == "JK":
            self._handle_jk_ff()
        elif self.ff_type == "SR":
            self._handle_sr_ff()
    
    def _handle_d_ff(self):
        """处理D触发器"""
        d_input = self.inputs["D"].state
        if d_input in [LogicState.LOW, LogicState.HIGH]:
            self.internal_state["Q"] = d_input
            self.outputs["Q"].set_state(d_input)
            self.outputs["Q_bar"].set_state(
                LogicState.HIGH if d_input == LogicState.LOW else LogicState.LOW
            )
    
    def _handle_jk_ff(self):
        """处理JK触发器"""
        j_input = self.inputs["J"].state
        k_input = self.inputs["K"].state
        current_q = self.internal_state["Q"]
        
        if j_input == LogicState.LOW and k_input == LogicState.LOW:
            # 保持状态
            pass
        elif j_input == LogicState.LOW and k_input == LogicState.HIGH:
            # 复位
            self.internal_state["Q"] = LogicState.LOW
        elif j_input == LogicState.HIGH and k_input == LogicState.LOW:
            # 置位
            self.internal_state["Q"] = LogicState.HIGH
        elif j_input == LogicState.HIGH and k_input == LogicState.HIGH:
            # 翻转
            self.internal_state["Q"] = LogicState.HIGH if current_q == LogicState.LOW else LogicState.LOW
        
        self.outputs["Q"].set_state(self.internal_state["Q"])
        self.outputs["Q_bar"].set_state(
            LogicState.HIGH if self.internal_state["Q"] == LogicState.LOW else LogicState.LOW
        )
    
    def _handle_sr_ff(self):
        """处理SR触发器"""
        s_input = self.inputs["S"].state
        r_input = self.inputs["R"].state
        
        if s_input == LogicState.LOW and r_input == LogicState.LOW:
            # 保持状态
            pass
        elif s_input == LogicState.LOW and r_input == LogicState.HIGH:
            # 复位
            self.internal_state["Q"] = LogicState.LOW
        elif s_input == LogicState.HIGH and r_input == LogicState.LOW:
            # 置位
            self.internal_state["Q"] = LogicState.HIGH
        else:
            # 非法状态
            self.internal_state["Q"] = LogicState.UNKNOWN
        
        self.outputs["Q"].set_state(self.internal_state["Q"])
        self.outputs["Q_bar"].set_state(
            LogicState.HIGH if self.internal_state["Q"] == LogicState.LOW else LogicState.LOW
        )
    
    def reset(self):
        """复位"""
        self.internal_state["Q"] = LogicState.LOW
        self.outputs["Q"].set_state(LogicState.LOW)
        self.outputs["Q_bar"].set_state(LogicState.HIGH)

class Counter(SequentialLogic):
    """计数器"""
    
    def __init__(self, name: str, width: int = 8, count_up: bool = True):
        super().__init__(name)
        self.width = width
        self.count_up = count_up
        self.max_count = (1 << width) - 1
        self.internal_state["count"] = 0
        
        # 添加输出信号
        for i in range(width):
            self.add_output(f"Q{i}")
        
        self.add_output("carry")
        self.add_output("zero")
    
    def on_clock_edge(self):
        """时钟边沿触发"""
        if self.reset_signal.state == LogicState.HIGH:
            self.reset()
            return
        
        if self.enable_signal.state == LogicState.LOW:
            return
        
        current_count = self.internal_state["count"]
        
        if self.count_up:
            new_count = (current_count + 1) % (self.max_count + 1)
            carry = (current_count == self.max_count)
        else:
            new_count = (current_count - 1) % (self.max_count + 1)
            carry = (current_count == 0)
        
        self.internal_state["count"] = new_count
        
        # 更新输出
        for i in range(self.width):
            bit_value = (new_count >> i) & 1
            self.outputs[f"Q{i}"].set_state(
                LogicState.HIGH if bit_value else LogicState.LOW
            )
        
        self.outputs["carry"].set_state(LogicState.HIGH if carry else LogicState.LOW)
        self.outputs["zero"].set_state(LogicState.HIGH if new_count == 0 else LogicState.LOW)
    
    def reset(self):
        """复位"""
        self.internal_state["count"] = 0
        for i in range(self.width):
            self.outputs[f"Q{i}"].set_state(LogicState.LOW)
        self.outputs["carry"].set_state(LogicState.LOW)
        self.outputs["zero"].set_state(LogicState.HIGH)

class ProcessorFSM(StateMachine):
    """处理器有限状态机"""
    
    def __init__(self, name: str):
        super().__init__(name)
        
        # 添加处理器状态
        self.add_state("FETCH")
        self.add_state("DECODE")
        self.add_state("EXECUTE")
        self.add_state("WRITEBACK")
        self.add_state("HALT")
        
        # 添加状态转移
        self.add_transition("IDLE", "FETCH", "start")
        self.add_transition("FETCH", "DECODE", "instruction_ready")
        self.add_transition("DECODE", "EXECUTE", "decoded")
        self.add_transition("EXECUTE", "WRITEBACK", "executed")
        self.add_transition("WRITEBACK", "FETCH", "written_back")
        self.add_transition("EXECUTE", "HALT", "halt_instruction")
        
        # 添加信号
        self.add_input("clock")
        self.add_input("reset")
        self.add_input("start")
        self.add_input("instruction_ready")
        self.add_input("decoded")
        self.add_input("executed")
        self.add_input("written_back")
        self.add_input("halt_instruction")
        
        self.add_output("fetch_enable")
        self.add_output("decode_enable")
        self.add_output("execute_enable")
        self.add_output("writeback_enable")
        self.add_output("halt")
    
    def process_inputs(self):
        """处理输入信号"""
        current_state = self.current_state
        
        # 根据当前状态和输入信号决定下一个状态
        if current_state == "IDLE" and self.input_signals["start"].state == LogicState.HIGH:
            self.transition_to("FETCH")
        elif current_state == "FETCH" and self.input_signals["instruction_ready"].state == LogicState.HIGH:
            self.transition_to("DECODE")
        elif current_state == "DECODE" and self.input_signals["decoded"].state == LogicState.HIGH:
            self.transition_to("EXECUTE")
        elif current_state == "EXECUTE":
            if self.input_signals["halt_instruction"].state == LogicState.HIGH:
                self.transition_to("HALT")
            elif self.input_signals["executed"].state == LogicState.HIGH:
                self.transition_to("WRITEBACK")
        elif current_state == "WRITEBACK" and self.input_signals["written_back"].state == LogicState.HIGH:
            self.transition_to("FETCH")
        
        # 更新输出信号
        self.update_outputs()
    
    def update_outputs(self):
        """更新输出信号"""
        # 清除所有输出
        for signal in self.output_signals.values():
            signal.set_state(LogicState.LOW)
        
        # 根据当前状态设置输出
        if self.current_state == "FETCH":
            self.output_signals["fetch_enable"].set_state(LogicState.HIGH)
        elif self.current_state == "DECODE":
            self.output_signals["decode_enable"].set_state(LogicState.HIGH)
        elif self.current_state == "EXECUTE":
            self.output_signals["execute_enable"].set_state(LogicState.HIGH)
        elif self.current_state == "WRITEBACK":
            self.output_signals["writeback_enable"].set_state(LogicState.HIGH)
        elif self.current_state == "HALT":
            self.output_signals["halt"].set_state(LogicState.HIGH)

# 使用示例
if __name__ == "__main__":
    print("=== 复杂逻辑电路系统演示 ===")
    
    # 创建D触发器
    print("\n--- D触发器演示 ---")
    d_ff = FlipFlop("D_FF_1", "D")
    d_ff.inputs["D"].set_state(LogicState.HIGH)
    d_ff.enable_signal.set_state(LogicState.HIGH)
    
    print(f"D输入: {d_ff.inputs['D'].state}")
    d_ff.on_clock_edge()
    print(f"Q输出: {d_ff.outputs['Q'].state}")
    print(f"Q_bar输出: {d_ff.outputs['Q_bar'].state}")
    
    # 创建计数器
    print("\n--- 4位计数器演示 ---")
    counter = Counter("Counter_4bit", width=4, count_up=True)
    counter.enable_signal.set_state(LogicState.HIGH)
    
    print("计数器状态:")
    for i in range(6):
        counter.on_clock_edge()
        count_value = counter.internal_state["count"]
        print(f"时钟{i+1}: 计数值={count_value}, 零标志={counter.outputs['zero'].state}")
    
    # 创建处理器状态机
    print("\n--- 处理器状态机演示 ---")
    cpu_fsm = ProcessorFSM("CPU_FSM")
    
    # 模拟处理器执行过程
    print("处理器状态转移:")
    cpu_fsm.input_signals["start"].set_state(LogicState.HIGH)
    cpu_fsm.process_inputs()
    
    cpu_fsm.input_signals["instruction_ready"].set_state(LogicState.HIGH)
    cpu_fsm.process_inputs()
    
    cpu_fsm.input_signals["decoded"].set_state(LogicState.HIGH)
    cpu_fsm.process_inputs()
    
    cpu_fsm.input_signals["executed"].set_state(LogicState.HIGH)
    cpu_fsm.process_inputs()
    
    cpu_fsm.input_signals["written_back"].set_state(LogicState.HIGH)
    cpu_fsm.process_inputs()
    
    print("\n=== 总结 ===")
    print("✓ 复杂逻辑电路使用类来处理时序和状态")
    print("✓ 基本组件（电阻、电容等）使用数据库存储")
    print("✓ 这种混合架构更适合EDA工具的实际需求")
    print("✓ 状态机和时序逻辑需要复杂的行为建模")
    print("✓ 数据库存储提供了更大的灵活性") 