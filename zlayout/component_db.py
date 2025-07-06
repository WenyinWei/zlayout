#!/usr/bin/env python3
"""
EDA组件数据库系统（简化版）
使用SQLite存储组件参数，支持灵活的组件定义和用户自定义模块
"""

import sqlite3
import json
import uuid
from typing import Dict, List, Any, Optional
from dataclasses import dataclass

@dataclass
class ComponentSpec:
    """组件规格定义"""
    name: str
    category: str  # 'passive', 'active', 'digital', 'analog', 'mixed', 'custom'
    parameters: Dict[str, Any]
    electrical_params: Dict[str, Any]
    physical_params: Dict[str, Any]
    technology_node: str = "28nm"
    manufacturer: str = "Generic"
    description: str = ""
    tags: Optional[List[str]] = None
    
    def __post_init__(self):
        if self.tags is None:
            self.tags = []

class ComponentDatabase:
    """组件数据库管理器"""
    
    def __init__(self, db_path: str = "components.db"):
        self.db_path = db_path
        self.conn = sqlite3.connect(self.db_path)
        self._init_database()
    
    def _init_database(self):
        """初始化数据库"""
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS components (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                category TEXT NOT NULL,
                parameters TEXT NOT NULL,
                electrical_params TEXT NOT NULL,
                physical_params TEXT NOT NULL,
                technology_node TEXT DEFAULT '28nm',
                manufacturer TEXT DEFAULT 'Generic',
                description TEXT DEFAULT '',
                tags TEXT DEFAULT '[]'
            )
        ''')
        
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS modules (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                description TEXT DEFAULT '',
                components TEXT NOT NULL,
                connections TEXT NOT NULL
            )
        ''')
        
        self.conn.commit()
        self._populate_basic_components()
    
    def _populate_basic_components(self):
        """填充基本组件库"""
        # 检查是否已有数据
        cursor = self.conn.execute("SELECT COUNT(*) FROM components")
        if cursor.fetchone()[0] > 0:
            return
        
        # 基本无源器件
        basic_components = [
            ComponentSpec(
                name="resistor_1k",
                category="passive",
                parameters={
                    "resistance": 1000,
                    "tolerance": 0.05,
                    "power_rating": 0.25
                },
                electrical_params={
                    "max_voltage": 200,
                    "temperature_coefficient": 100
                },
                physical_params={
                    "package": "0603",
                    "area": 1.28  # mm²
                },
                description="1kΩ电阻",
                tags=["passive", "resistor"]
            ),
            ComponentSpec(
                name="capacitor_100nf",
                category="passive", 
                parameters={
                    "capacitance": 100e-9,
                    "tolerance": 0.1,
                    "voltage_rating": 25
                },
                electrical_params={
                    "esr": 0.01,
                    "temperature_range": [-55, 125]
                },
                physical_params={
                    "package": "0402",
                    "area": 0.5  # mm²
                },
                description="100nF陶瓷电容",
                tags=["passive", "capacitor"]
            ),
            ComponentSpec(
                name="inductor_10uh",
                category="passive",
                parameters={
                    "inductance": 10e-6,
                    "tolerance": 0.2,
                    "current_rating": 1.0
                },
                electrical_params={
                    "dc_resistance": 0.1,
                    "q_factor": 50
                },
                physical_params={
                    "package": "0603",
                    "area": 1.28  # mm²
                },
                description="10μH电感",
                tags=["passive", "inductor"]
            )
        ]
        
        for comp in basic_components:
            self.add_component(comp)
    
    def add_component(self, spec: ComponentSpec) -> str:
        """添加组件到数据库"""
        component_id = str(uuid.uuid4())
        
        self.conn.execute('''
            INSERT INTO components (id, name, category, parameters, electrical_params, 
                                  physical_params, technology_node, manufacturer, description, tags)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            component_id,
            spec.name,
            spec.category,
            json.dumps(spec.parameters),
            json.dumps(spec.electrical_params),
            json.dumps(spec.physical_params),
            spec.technology_node,
            spec.manufacturer,
            spec.description,
            json.dumps(spec.tags)
        ))
        
        self.conn.commit()
        return component_id
    
    def get_component(self, component_id: str) -> Optional[ComponentSpec]:
        """获取组件规格"""
        cursor = self.conn.execute(
            "SELECT * FROM components WHERE id = ?", (component_id,)
        )
        row = cursor.fetchone()
        
        if not row:
            return None
        
        return ComponentSpec(
            name=row[1],
            category=row[2],
            parameters=json.loads(row[3]),
            electrical_params=json.loads(row[4]),
            physical_params=json.loads(row[5]),
            technology_node=row[6],
            manufacturer=row[7],
            description=row[8],
            tags=json.loads(row[9])
        )
    
    def search_components(self, 
                         category: Optional[str] = None,
                         name_pattern: Optional[str] = None) -> List[Dict[str, Any]]:
        """搜索组件"""
        query = "SELECT id, name, category, description FROM components WHERE 1=1"
        params = []
        
        if category:
            query += " AND category = ?"
            params.append(category)
        
        if name_pattern:
            query += " AND name LIKE ?"
            params.append(f"%{name_pattern}%")
        
        cursor = self.conn.execute(query, params)
        results = []
        
        for row in cursor.fetchall():
            results.append({
                "id": row[0],
                "name": row[1],
                "category": row[2],
                "description": row[3]
            })
        
        return results
    
    def create_custom_component(self, 
                              name: str,
                              parameters: Dict[str, Any],
                              electrical_params: Optional[Dict[str, Any]] = None,
                              physical_params: Optional[Dict[str, Any]] = None,
                              description: str = "") -> str:
        """创建自定义组件"""
        spec = ComponentSpec(
            name=name,
            category="custom",
            parameters=parameters,
            electrical_params=electrical_params if electrical_params else {},
            physical_params=physical_params if physical_params else {},
            description=description,
            tags=["custom"]
        )
        
        return self.add_component(spec)
    
    def create_module(self,
                     name: str,
                     components: List[str],
                     connections: List[Dict[str, Any]],
                     description: str = "") -> str:
        """创建模块（包括中间模块）"""
        module_id = str(uuid.uuid4())
        
        # 如果没有名称，生成一个
        if not name:
            name = f"module_{module_id[:8]}"
        
        self.conn.execute('''
            INSERT INTO modules (id, name, description, components, connections)
            VALUES (?, ?, ?, ?, ?)
        ''', (
            module_id,
            name,
            description,
            json.dumps(components),
            json.dumps(connections)
        ))
        
        self.conn.commit()
        return module_id
    
    def get_component_library(self) -> Dict[str, Any]:
        """获取组件库概览"""
        cursor = self.conn.execute('''
            SELECT category, COUNT(*) as count
            FROM components 
            GROUP BY category
        ''')
        
        library = {}
        for row in cursor.fetchall():
            category, count = row
            library[category] = count
        
        return library
    
    def close(self):
        """关闭数据库连接"""
        if self.conn:
            self.conn.close()

# 使用示例
if __name__ == "__main__":
    # 创建数据库
    db = ComponentDatabase("eda_components.db")
    
    # 查看基本组件库
    print("=== 基本组件库 ===")
    library = db.get_component_library()
    for category, count in library.items():
        print(f"{category}: {count} 个组件")
    
    # 搜索组件
    print("\n=== 搜索无源器件 ===")
    passive_components = db.search_components(category="passive")
    for comp in passive_components:
        print(f"- {comp['name']}: {comp['description']}")
    
    # 创建自定义组件
    print("\n=== 创建自定义组件 ===")
    custom_id = db.create_custom_component(
        name="custom_op_amp",
        parameters={
            "gain": 100,
            "bandwidth": 1e6,
            "slew_rate": 1e6
        },
        electrical_params={
            "supply_voltage": [3.3, 5.0],
            "input_impedance": 1e12
        },
        description="自定义运算放大器"
    )
    print(f"创建自定义组件ID: {custom_id}")
    
    # 创建中间模块
    print("\n=== 创建中间模块 ===")
    module_id = db.create_module(
        name="amplifier_stage",
        components=[custom_id],
        connections=[{"from": "input", "to": "op_amp_in"}],
        description="放大器级"
    )
    print(f"创建模块ID: {module_id}")
    
    # 获取组件详情
    print("\n=== 组件详情 ===")
    comp = db.get_component(custom_id)
    if comp:
        print(f"组件名: {comp.name}")
        print(f"类别: {comp.category}")
        print(f"参数: {comp.parameters}")
    
    db.close() 