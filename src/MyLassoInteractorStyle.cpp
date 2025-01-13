#include "MyLassoInteractorStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellPicker.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(MyLassoInteractorStyle);

void MyLassoInteractorStyle::SetPicker(vtkSmartPointer<vtkCellPicker> picker)
{
    this->Picker = picker;
}

void MyLassoInteractorStyle::SetRenderer(vtkSmartPointer<vtkRenderer> renderer)
{
    this->Renderer = renderer;
}

void MyLassoInteractorStyle::SetMesh(vtkSmartPointer<vtkPolyData> mesh)
{
    this->Mesh = mesh;
}

void MyLassoInteractorStyle::SetCellColors(vtkSmartPointer<vtkUnsignedCharArray> cellColors)
{
    this->CellColors = cellColors;
}

void MyLassoInteractorStyle::SetSelectedCells(vtkSmartPointer<vtkIdList> selectedCells)
{
    this->SelectedCells = selectedCells;
}

void MyLassoInteractorStyle::OnLeftButtonDown()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        return;
    }

    // ������ ��������� ������� ����
    this->IsDragging = false;
    this->GetInteractor()->GetEventPosition(this->StartPosition);
}

void MyLassoInteractorStyle::OnMouseMove()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        return;
    }

    // ��������, �������� �� ��������������
    if (!IsDragging) {
        int currentPosition[2];
        this->GetInteractor()->GetEventPosition(currentPosition);

        if (std::abs(currentPosition[0] - StartPosition[0]) > 5 ||
            std::abs(currentPosition[1] - StartPosition[1]) > 5) {
            this->IsDragging = true;
        }
    }
}

void MyLassoInteractorStyle::OnLeftButtonUp()
{
    if (!Picker || !Renderer || !Mesh || !CellColors || !SelectedCells) {
        return;
    }

    // ���� �������������� �� ��������, ��������� ���������
    if (!IsDragging) {
        int* clickPos = this->GetInteractor()->GetEventPosition();
        Picker->Pick(clickPos[0], clickPos[1], 0, Renderer);
        vtkIdType cellId = Picker->GetCellId();

        if (cellId >= 0) {
            // �������� ������ (������ � ���� �� �������)
            CellColors->SetTuple3(cellId, 255, 0, 0);
            Mesh->GetCellData()->SetScalars(CellColors);
            Mesh->Modified();
            Renderer->GetRenderWindow()->Render();
            SelectedCells->InsertUniqueId(cellId);
        }
    }
}

void MyLassoInteractorStyle::OnMouseWheelBackward()
{
    // ��������� ����������� ��������� ��������������� �����
    // ������ �� ������, ����� �������� ���� �� ������ �� ���������������
}

void MyLassoInteractorStyle::OnMouseWheelForward()
{
    // ��������� ����������� ��������� ��������������� �����
    // ������ �� ������, ����� �������� ���� �� ������ �� ���������������
}