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

#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkPolyDataNormals.h>

#include <vtkFloatArray.h>

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

    // �������� ������� ���� ����������
    int* windowSize = this->Renderer->GetRenderWindow()->GetSize();

    // ����������� Z-�����
    vtkSmartPointer<vtkFloatArray> zBuffer = vtkSmartPointer<vtkFloatArray>::New();
    this->Renderer->GetRenderWindow()->GetZbufferData(0, 0, windowSize[0] - 1, windowSize[1] - 1, zBuffer);
    if (!zBuffer) {
        std::cerr << "������: �� ������� �������� Z-�����." << std::endl;
        return;
    }

    // ����������� Z-����� � ������
    float* zBufferData = zBuffer->GetPointer(0);

    // �������� ��������� ������
    vtkCamera* camera = Renderer->GetActiveCamera();
    double nearPlane = camera->GetClippingRange()[0];
    double farPlane = camera->GetClippingRange()[1];

    // ��������� ����� ������
    double bounds[6];
    Mesh->GetBounds(bounds);
    double modelCenter[3] = {
        (bounds[0] + bounds[1]) / 2.0,
        (bounds[2] + bounds[3]) / 2.0,
        (bounds[4] + bounds[5]) / 2.0
    };

    // ��������� ���������� �� ������ �� ������ ������
    double cameraPosition[3];
    camera->GetPosition(cameraPosition);
    double distanceToModel = sqrt(pow(cameraPosition[0] - modelCenter[0], 2) +
        pow(cameraPosition[1] - modelCenter[1], 2) +
        pow(cameraPosition[2] - modelCenter[2], 2));

    // ������������� ������������ �������� tolerance
    double k = 0.00001; // ����������� ���������������
    double tolerance = k * distanceToModel;

    // ���������� �������� �����, ���������� � ��������� �������
    for (vtkIdType cellId = 0; cellId < Mesh->GetNumberOfCells(); ++cellId) {
        double bounds[6];
        Mesh->GetCellBounds(cellId, bounds);

        // ��������� ����� ������
        double center[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

        // ����������� ����� �������� � �������� ����������
        double displayCoord[3];
        this->Renderer->SetWorldPoint(center[0], center[1], center[2], 1.0);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(displayCoord);

        // ���������, ��������� �� ����� � �������� ����
        int screenX = static_cast<int>(displayCoord[0]);
        int screenY = static_cast<int>(displayCoord[1]);
        if (screenX < 0 || screenX >= windowSize[0] || screenY < 0 || screenY >= windowSize[1]) {
            continue;
        }

        // �������� �������� ������� �� Z-������
        int zIndex = screenY * windowSize[0] + screenX;
        double zBufferValue = zBufferData[zIndex];

        // ��������� ��������� ��������
        if (displayCoord[2] - zBufferValue > tolerance) {
            continue; // ������� �� �����
        }

        // ���������, �������� �� ����� ������ � ��������� �������
        if (IsPointInPolygon(displayCoord[0], displayCoord[1], LassoPoints)) {
            this->SelectedCells->InsertUniqueId(cellId);
        }
    }

    // ������������ ��������� ��������
    for (vtkIdType i = 0; i < this->SelectedCells->GetNumberOfIds(); ++i) {
        vtkIdType cellId = this->SelectedCells->GetId(i);
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