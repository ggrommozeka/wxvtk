#ifndef MYLASSOINTERACTORSTYLE_H
#define MYLASSOINTERACTORSTYLE_H

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkCellPicker.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdList.h>
#include <vector>
#include <array>

class MyLassoInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyLassoInteractorStyle* New();
    vtkTypeMacro(MyLassoInteractorStyle, vtkInteractorStyleTrackballCamera);

    void SetPicker(vtkSmartPointer<vtkCellPicker> picker);
    void SetRenderer(vtkSmartPointer<vtkRenderer> renderer);
    void SetMesh(vtkSmartPointer<vtkPolyData> mesh);
    void SetCellColors(vtkSmartPointer<vtkUnsignedCharArray> cellColors);
    void SetSelectedCells(vtkSmartPointer<vtkIdList> selectedCells);

protected:
    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

private:
	void DrawLasso(); // to do
    bool IsPointInPolygon(double x, double y, const std::vector<std::array<int, 2>>& polygon);

    vtkSmartPointer<vtkCellPicker> Picker;
    vtkSmartPointer<vtkRenderer> Renderer;
    vtkSmartPointer<vtkPolyData> Mesh;
    vtkSmartPointer<vtkUnsignedCharArray> CellColors;
    vtkSmartPointer<vtkIdList> SelectedCells;
    bool IsLassoActive = false; 
    std::vector<std::array<int, 2>> LassoPoints; 
};

#endif // MYLASSOINTERACTORSTYLE_H