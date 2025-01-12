#ifndef MYINTERACTORSTYLE_H
#define MYINTERACTORSTYLE_H

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkCellPicker.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdList.h>

#include <vtkRenderWindowInteractor.h> 
#include <vtkCellData.h>               
#include <vtkRenderWindow.h>           

class MyInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyInteractorStyle* New();
    vtkTypeMacro(MyInteractorStyle, vtkInteractorStyleTrackballCamera);

    void SetPicker(vtkSmartPointer<vtkCellPicker> picker);
    void SetRenderer(vtkSmartPointer<vtkRenderer> renderer);
    void SetMesh(vtkSmartPointer<vtkPolyData> mesh);
    void SetCellColors(vtkSmartPointer<vtkUnsignedCharArray> cellColors);
    void SetSelectedCells(vtkSmartPointer<vtkIdList> selectedCells);

protected:
    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;
    void OnRightButtonUp() override;

private:
    vtkSmartPointer<vtkCellPicker> Picker;
    vtkSmartPointer<vtkRenderer> Renderer;
    vtkSmartPointer<vtkPolyData> Mesh;
    vtkSmartPointer<vtkUnsignedCharArray> CellColors;
    vtkSmartPointer<vtkIdList> SelectedCells;
    bool IsDragging = false;
    int StartPosition[2];
};

#endif // MYINTERACTORSTYLE_H