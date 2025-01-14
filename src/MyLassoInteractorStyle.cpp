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

	// clear previous lasso points
    LassoPoints.clear();

	// grab the initial mouse position
    int startPos[2];
    this->GetInteractor()->GetEventPosition(startPos);
    LassoPoints.push_back({ startPos[0], startPos[1] });

	this->IsLassoActive = true; // activate lasso
}

void MyLassoInteractorStyle::OnMouseMove()
{
    if (!IsLassoActive) {
        return;
    }

	// grab the current mouse position
    int currentPos[2];
    this->GetInteractor()->GetEventPosition(currentPos);
    LassoPoints.push_back({ currentPos[0], currentPos[1] });

	// draw the lasso trajectory
    this->DrawLasso();

	// render the scene
    this->GetInteractor()->Render();
}

void MyLassoInteractorStyle::OnLeftButtonUp()
{
    if (!IsLassoActive) {
        return;
    }

	// deactivate lasso
    this->IsLassoActive = false;

	// get the rendering window size
    int* windowSize = this->Renderer->GetRenderWindow()->GetSize();

	// grab the Z-buffer
    vtkSmartPointer<vtkFloatArray> zBuffer = vtkSmartPointer<vtkFloatArray>::New();
    this->Renderer->GetRenderWindow()->GetZbufferData(0, 0, windowSize[0] - 1, windowSize[1] - 1, zBuffer);
    if (!zBuffer) {
        std::cerr << "Error: failed to get Z-buffer" << std::endl;
        return;
    }

	// convert Z-buffer to an array
    float* zBufferData = zBuffer->GetPointer(0);

	// get camera parameters
    vtkCamera* camera = Renderer->GetActiveCamera();
    double nearPlane = camera->GetClippingRange()[0];
    double farPlane = camera->GetClippingRange()[1];

    // get model bounding box
    double bounds[6];
    Mesh->GetBounds(bounds);

	// calculate the distance from the camera to the bounding box
    double cameraPosition[3];
    camera->GetPosition(cameraPosition);

	// calculate the distance from the camera to the bounding box
    double minDistanceToBounds = std::numeric_limits<double>::max();

    for (int i = 0; i < 8; ++i) {
        // bounding box vertices
        double corner[3];
        if (i % 2 == 0) {
            corner[0] = bounds[0]; // x -> minX
        }
        else {
            corner[0] = bounds[1]; // x -> maxX
        }

        if ((i / 2) % 2 == 0) {
            corner[1] = bounds[2]; // y -> minY
        }
        else {
            corner[1] = bounds[3]; // y -> maxY
        }

        if ((i / 4) % 2 == 0) {
            corner[2] = bounds[4]; // z -> minZ
        }
        else {
            corner[2] = bounds[5]; // z -> maxZ
        }

		// calculate the distance from the camera to the current corner
        double distance = sqrt(
            pow(cameraPosition[0] - corner[0], 2) +
            pow(cameraPosition[1] - corner[1], 2) +
            pow(cameraPosition[2] - corner[2], 2)
        );

        if (distance < minDistanceToBounds) {
            minDistanceToBounds = distance;
        }
    }

    // set dynamic tolerance
    double k = 0.00001; // scale factor
    double tolerance = k * minDistanceToBounds;

	// find selected triangle cells using centroid  
    for (vtkIdType cellId = 0; cellId < Mesh->GetNumberOfCells(); ++cellId) {
        double bounds[6];
        Mesh->GetCellBounds(cellId, bounds);

		// calculate centroid of the cell
        double center[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

		// convert world point to display point
        double displayCoord[3];
        this->Renderer->SetWorldPoint(center[0], center[1], center[2], 1.0);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(displayCoord);

		// check if the point is within the window
        int screenX = static_cast<int>(displayCoord[0]);
        int screenY = static_cast<int>(displayCoord[1]);
        if (screenX < 0 || screenX >= windowSize[0] || screenY < 0 || screenY >= windowSize[1]) {
            continue;
        }

		// get Z-buffer value
        int zIndex = screenY * windowSize[0] + screenX;
        double zBufferValue = zBufferData[zIndex];

        if (displayCoord[2] - zBufferValue > tolerance) {
			continue; // cell is occluded
        }

        if (IsPointInPolygon(displayCoord[0], displayCoord[1], LassoPoints)) {
            this->SelectedCells->InsertUniqueId(cellId);
        }
    }

	// highlight selected cells
    for (vtkIdType i = 0; i < this->SelectedCells->GetNumberOfIds(); ++i) {
        vtkIdType cellId = this->SelectedCells->GetId(i);
        CellColors->SetTuple3(cellId, 255, 0, 0);
    }

    Mesh->GetCellData()->SetScalars(CellColors);
    Mesh->Modified();
    Renderer->GetRenderWindow()->Render();
}


void MyLassoInteractorStyle::OnMouseWheelForward()
{
	// block scene scaling
}

void MyLassoInteractorStyle::OnMouseWheelBackward()
{
    // block scene scaling
}

void MyLassoInteractorStyle::DrawLasso()
{
    if (!IsLassoActive || LassoPoints.empty()) {
        return;
    }

	// to do: draw lasso
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