"""
Layout visualization and plotting utilities.
"""

try:
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    plt = None
    patches = None

from typing import List, Dict, Optional, Tuple
from .geometry import Point, Rectangle, Polygon


class LayoutVisualizer:
    """Visualizer for layout analysis results."""
    
    def __init__(self, figsize: Tuple[float, float] = (12, 8)):
        if not HAS_MATPLOTLIB:
            raise ImportError("Matplotlib is required for visualization. Install with: pip install matplotlib")
        self.figsize = figsize
        
    def plot_layout(self, 
                   polygons: List[Polygon], 
                   rectangles: List[Rectangle] = None,
                   title: str = "Layout View"):
        """Plot polygons and rectangles in a layout."""
        fig, ax = plt.subplots(figsize=self.figsize)
        
        # Plot polygons
        for i, polygon in enumerate(polygons):
            x_coords = [v.x for v in polygon.vertices] + [polygon.vertices[0].x]
            y_coords = [v.y for v in polygon.vertices] + [polygon.vertices[0].y]
            
            ax.plot(x_coords, y_coords, 'b-', linewidth=2, alpha=0.7)
            ax.fill(x_coords, y_coords, alpha=0.3, color='lightblue')
            
            # Add polygon ID label
            centroid = self._calculate_centroid(polygon)
            ax.text(centroid.x, centroid.y, f'P{i}', 
                   ha='center', va='center', fontweight='bold')
        
        # Plot rectangles
        if rectangles:
            for i, rect in enumerate(rectangles):
                rect_patch = patches.Rectangle(
                    (rect.x, rect.y), rect.width, rect.height,
                    linewidth=2, edgecolor='red', facecolor='lightcoral', alpha=0.3
                )
                ax.add_patch(rect_patch)
                
                # Add rectangle ID label
                ax.text(rect.center.x, rect.center.y, f'R{i}',
                       ha='center', va='center', fontweight='bold')
        
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        ax.set_title(title)
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        
        return fig
    
    def plot_analysis_results(self, 
                            polygons: List[Polygon],
                            analysis_results: Dict,
                            title: str = "Layout Analysis"):
        """Plot layout with analysis overlays."""
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
        
        # Plot 1: Original layout
        self._plot_polygons_on_axis(ax1, polygons, "Original Layout")
        
        # Plot 2: Sharp angles
        self._plot_polygons_on_axis(ax2, polygons, "Sharp Angles")
        sharp_angles = analysis_results.get('sharp_angles', {}).get('details', [])
        for poly_id, vertex_idx, angle in sharp_angles:
            if poly_id < len(polygons):
                vertex = polygons[poly_id].vertices[vertex_idx]
                ax2.plot(vertex.x, vertex.y, 'ro', markersize=8)
                ax2.text(vertex.x, vertex.y, f'{angle:.1f}°', 
                        xytext=(5, 5), textcoords='offset points', 
                        fontsize=8, color='red')
        
        # Plot 3: Narrow distances
        self._plot_polygons_on_axis(ax3, polygons, "Narrow Distances")
        narrow_regions = analysis_results.get('narrow_distances', {}).get('details', [])
        for point1, point2, distance in narrow_regions:
            ax3.plot([point1.x, point2.x], [point1.y, point2.y], 
                    'r-', linewidth=3, alpha=0.7)
            mid_x = (point1.x + point2.x) / 2
            mid_y = (point1.y + point2.y) / 2
            ax3.text(mid_x, mid_y, f'{distance:.2f}', 
                    ha='center', va='center', fontsize=8, 
                    bbox=dict(boxstyle='round,pad=0.2', facecolor='yellow', alpha=0.7))
        
        # Plot 4: Intersections
        self._plot_polygons_on_axis(ax4, polygons, "Edge Intersections")
        intersection_points = analysis_results.get('intersections', {}).get('points', [])
        for point in intersection_points:
            ax4.plot(point.x, point.y, 'rx', markersize=10, markeredgewidth=3)
        
        plt.tight_layout()
        return fig
    
    def plot_quadtree_visualization(self, 
                                  quadtree_node, 
                                  polygons: List[Polygon] = None,
                                  title: str = "QuadTree Structure"):
        """Visualize quadtree structure."""
        fig, ax = plt.subplots(figsize=self.figsize)
        
        # Plot polygons if provided
        if polygons:
            for polygon in polygons:
                x_coords = [v.x for v in polygon.vertices] + [polygon.vertices[0].x]
                y_coords = [v.y for v in polygon.vertices] + [polygon.vertices[0].y]
                ax.plot(x_coords, y_coords, 'b-', linewidth=1, alpha=0.5)
                ax.fill(x_coords, y_coords, alpha=0.2, color='lightblue')
        
        # Recursively draw quadtree boundaries
        self._draw_quadtree_boundaries(ax, quadtree_node, depth=0)
        
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        ax.set_title(title)
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        
        return fig
    
    def plot_optimization_summary(self, optimization_results: Dict):
        """Create a summary visualization of optimization results."""
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
        
        analysis = optimization_results['analysis']
        
        # Plot 1: Score gauge
        score = optimization_results['optimization_score']
        self._plot_score_gauge(ax1, score)
        
        # Plot 2: Issue counts
        issue_types = ['Sharp Angles', 'Narrow Distances', 'Intersections']
        counts = [
            analysis['sharp_angles']['count'],
            analysis['narrow_distances']['count'],
            analysis['intersections']['polygon_pairs']
        ]
        colors = ['orange', 'red', 'darkred']
        
        bars = ax2.bar(issue_types, counts, color=colors, alpha=0.7)
        ax2.set_title('Issues Found')
        ax2.set_ylabel('Count')
        
        # Add value labels on bars
        for bar, count in zip(bars, counts):
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{count}', ha='center', va='bottom')
        
        # Plot 3: Distance statistics
        if analysis['narrow_distances']['count'] > 0:
            min_dist = analysis['narrow_distances']['minimum']
            max_dist = analysis['narrow_distances']['maximum']
            avg_dist = analysis['narrow_distances']['average']
            
            distances = ['Minimum', 'Average', 'Maximum']
            values = [min_dist, avg_dist, max_dist]
            
            ax3.bar(distances, values, color='skyblue', alpha=0.7)
            ax3.set_title('Distance Statistics')
            ax3.set_ylabel('Distance')
            
            for bar, value in zip(ax3.patches, values):
                height = bar.get_height()
                ax3.text(bar.get_x() + bar.get_width()/2., height,
                        f'{value:.3f}', ha='center', va='bottom')
        else:
            ax3.text(0.5, 0.5, 'No narrow distances found', 
                    ha='center', va='center', transform=ax3.transAxes)
            ax3.set_title('Distance Statistics')
        
        # Plot 4: Suggestions
        suggestions = optimization_results['suggestions']
        ax4.axis('off')
        ax4.set_title('Optimization Suggestions')
        
        if suggestions:
            suggestion_text = '\n\n'.join([f"• {s}" for s in suggestions])
        else:
            suggestion_text = "✓ No issues found! Layout looks good."
        
        ax4.text(0.05, 0.95, suggestion_text, transform=ax4.transAxes,
                verticalalignment='top', fontsize=10, wrap=True)
        
        plt.tight_layout()
        return fig
    
    def _plot_polygons_on_axis(self, ax, polygons: List[Polygon], title: str):
        """Helper to plot polygons on a given axis."""
        for polygon in polygons:
            x_coords = [v.x for v in polygon.vertices] + [polygon.vertices[0].x]
            y_coords = [v.y for v in polygon.vertices] + [polygon.vertices[0].y]
            
            ax.plot(x_coords, y_coords, 'b-', linewidth=2, alpha=0.7)
            ax.fill(x_coords, y_coords, alpha=0.3, color='lightblue')
        
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        ax.set_title(title)
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
    
    def _draw_quadtree_boundaries(self, ax, node, depth: int = 0):
        """Recursively draw quadtree node boundaries."""
        if depth > 6:  # Prevent too deep recursion for visualization
            return
            
        # Draw this node's boundary
        boundary = node.boundary
        rect = patches.Rectangle(
            (boundary.x, boundary.y), boundary.width, boundary.height,
            linewidth=1, edgecolor='gray', facecolor='none', alpha=0.5
        )
        ax.add_patch(rect)
        
        # Add object count text
        object_count = len(node.objects)
        if object_count > 0:
            center = boundary.center
            ax.text(center.x, center.y, str(object_count),
                   ha='center', va='center', fontsize=8, 
                   bbox=dict(boxstyle='round,pad=0.2', facecolor='white', alpha=0.7))
        
        # Recursively draw children
        if node.divided:
            for child in node.children:
                self._draw_quadtree_boundaries(ax, child, depth + 1)
    
    def _plot_score_gauge(self, ax, score: float):
        """Plot a gauge showing optimization score."""
        # Create a semicircle gauge
        theta = (score / 100) * 180  # Convert score to degrees (0-180)
        
        # Background arc
        import math
        theta_bg = list(range(0, 181, 5))
        x_bg = [0.8 * math.cos(math.radians(t)) for t in theta_bg]
        y_bg = [0.8 * math.sin(math.radians(t)) for t in theta_bg]
        ax.plot(x_bg, y_bg, 'lightgray', linewidth=10)
        
        # Score arc
        import math
        theta_score = list(range(0, int(theta) + 1, 5))
        x_score = [0.8 * math.cos(math.radians(t)) for t in theta_score]
        y_score = [0.8 * math.sin(math.radians(t)) for t in theta_score]
        
        # Color based on score
        if score >= 80:
            color = 'green'
        elif score >= 60:
            color = 'orange'
        else:
            color = 'red'
        
        if x_score:  # Only plot if there are points
            ax.plot(x_score, y_score, color, linewidth=10)
        
        # Score text
        ax.text(0, -0.3, f'{score:.1f}', ha='center', va='center', 
               fontsize=24, fontweight='bold')
        ax.text(0, -0.5, 'Optimization Score', ha='center', va='center', 
               fontsize=12)
        
        ax.set_xlim(-1, 1)
        ax.set_ylim(-0.7, 1)
        ax.set_aspect('equal')
        ax.axis('off')
        ax.set_title('Layout Quality Score')
    
    def _calculate_centroid(self, polygon: Polygon) -> Point:
        """Calculate polygon centroid."""
        x_sum = sum(v.x for v in polygon.vertices)
        y_sum = sum(v.y for v in polygon.vertices)
        n = len(polygon.vertices)
        return Point(x_sum / n, y_sum / n)
    
    def save_plots(self, figures: List, base_filename: str, 
                  format: str = 'png', dpi: int = 300):
        """Save multiple figures to files."""
        for i, fig in enumerate(figures):
            filename = f"{base_filename}_{i+1}.{format}"
            fig.savefig(filename, format=format, dpi=dpi, bbox_inches='tight')
            print(f"Saved plot: {filename}")