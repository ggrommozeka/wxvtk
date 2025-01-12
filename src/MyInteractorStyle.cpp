#include "MyInteractorStyle.h"

vtkStandardNewMacro(MyInteractorStyle);

void MyInteractorStyle::SetPicker(vtkSmartPointer<vtkCellPicker> picker)
{
    this->Picker = picker;
}

void MyInteractorStyle::SetRenderer(vtkSmartPointer<vtkRenderer> renderer)
{
    this->Renderer = renderer;
}

void MyInteractorStyle::SetMesh(vtkSmartPointer<vtkPolyData> mesh)
{
    this->Mesh = mesh;
}

void MyInteractorStyle::SetCellColors(vtkSmartPointer<vtkUnsignedCharArray> cellColors)
{
    this->CellColors = cellColors;
}

void MyInteractorStyle::SetSelectedCells(vtkSmartPointer<vtkIdList> selectedCells)
{
    this->SelectedCells = selectedCells;
}

void MyInteractorStyle::OnLeftButtonDown()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
        return;
    }

    this->IsDragging = false;
    this->GetInteractor()->GetEventPosition(this->StartPosition);

    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void MyInteractorStyle::OnMouseMove()
{
    if (!IsDragging) {
        int currentPosition[2];
        this->GetInteractor()->GetEventPosition(currentPosition);

        if (std::abs(currentPosition[0] - StartPosition[0]) > 5 ||
            std::abs(currentPosition[1] - StartPosition[1]) > 5) {
            this->IsDragging = true;
        }
    }

    vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void MyInteractorStyle::OnLeftButtonUp()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
        return;
    }

    if (!IsDragging) {
        int* clickPos = this->GetInteractor()->GetEventPosition();
        Picker->Pick(clickPos[0], clickPos[1], 0, Renderer);
        vtkIdType cellId = Picker->GetCellId();

        if (cellId >= 0) {
            CellColors->SetTuple3(cellId, 255, 0, 0);
            Mesh->GetCellData()->SetScalars(CellColors);
            Mesh->Modified();
            Renderer->GetRenderWindow()->Render();
            SelectedCells->InsertUniqueId(cellId);
        }
    }

    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

void MyInteractorStyle::OnRightButtonUp()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        vtkInteractorStyleTrackballCamera::OnRightButtonUp();
        return;
    }

    int* clickPos = this->GetInteractor()->GetEventPosition();
    Picker->Pick(clickPos[0], clickPos[1], 0, Renderer);
    vtkIdType cellId = Picker->GetCellId();

    if (cellId >= 0) {
        CellColors->SetTuple3(cellId, 255, 255, 255);
        Mesh->GetCellData()->SetScalars(CellColors);
        Mesh->Modified();
        Renderer->GetRenderWindow()->Render();
        SelectedCells->DeleteId(cellId);
    }

    vtkInteractorStyleTrackballCamera::OnRightButtonUp();
}