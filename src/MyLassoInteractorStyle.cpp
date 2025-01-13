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

    // ������� ���������� ����� �����
    LassoPoints.clear();

    // ������ ��������� ������� ����
    int startPos[2];
    this->GetInteractor()->GetEventPosition(startPos);
    LassoPoints.push_back({ startPos[0], startPos[1] });

    this->IsLassoActive = true; // ���������� �����
}

void MyLassoInteractorStyle::OnMouseMove()
{
    if (!IsLassoActive) {
        return;
    }

    // ������ ������� ������� ����
    int currentPos[2];
    this->GetInteractor()->GetEventPosition(currentPos);
    LassoPoints.push_back({ currentPos[0], currentPos[1] });

    // ������������ ���������� �����
    this->DrawLasso();

    // �������������� �����
    this->GetInteractor()->Render();
}

void MyLassoInteractorStyle::OnLeftButtonUp()
{
    if (!IsLassoActive) {
        return;
    }

    // ������������ �����
    this->IsLassoActive = false;

    // ���������� �������� �����, ���������� � ��������� �������
    vtkSmartPointer<vtkIdList> selectedCells = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType cellId = 0; cellId < Mesh->GetNumberOfCells(); ++cellId) {
        double bounds[6];
        Mesh->GetCellBounds(cellId, bounds);

        // ����������� ������� ������ � �������� ����������
        double center[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };
        double displayCoord[3];
        this->Renderer->SetWorldPoint(center[0], center[1], center[2], 1.0);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(displayCoord);

        // ���������, �������� �� ����� ������ � ��������� �������
        if (IsPointInPolygon(displayCoord[0], displayCoord[1], LassoPoints)) {
            selectedCells->InsertNextId(cellId);
        }
    }

    // ������������ ��������� ��������
    for (vtkIdType i = 0; i < selectedCells->GetNumberOfIds(); ++i) {
        vtkIdType cellId = selectedCells->GetId(i);
        CellColors->SetTuple3(cellId, 255, 0, 0); // ������� ���� ��� ���������� ���������
    }

    Mesh->GetCellData()->SetScalars(CellColors);
    Mesh->Modified();
    Renderer->GetRenderWindow()->Render();
}

void MyLassoInteractorStyle::OnMouseWheelForward()
{
    // ��������� ����������� ��������� ��������������� �����
}

void MyLassoInteractorStyle::OnMouseWheelBackward()
{
    // ��������� ����������� ��������� ��������������� �����
}

void MyLassoInteractorStyle::DrawLasso()
{
    if (!IsLassoActive || LassoPoints.empty()) {
        return;
    }

    // ������������� ���� � ������� �����
    //glColor3f(1.0, 0.0, 0.0); // ������� ����
    //glLineWidth(2.0);         // ������� �����

    //// ������ ���������� �����
    //glBegin(GL_LINE_STRIP);
    //for (const auto& point : LassoPoints) {
    //    glVertex2i(point[0], point[1]);
    //}
    //glEnd();
}

bool MyLassoInteractorStyle::IsPointInPolygon(double x, double y, const std::vector<std::array<int, 2>>& polygon)
{
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i][1] > y) != (polygon[j][1] > y)) &&
            (x < (polygon[j][0] - polygon[i][0]) * (y - polygon[i][1]) / (polygon[j][1] - polygon[i][1]) + polygon[i][0])) {
            inside = !inside;
        }
    }
    return inside;
}