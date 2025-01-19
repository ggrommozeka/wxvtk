#include "MyLassoInteractorStyle.h"
#include <wx/msgdlg.h>

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

#include <vtkHardwareSelector.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>

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

    // !hardware selector instead Z-buffer
    vtkSmartPointer<vtkHardwareSelector> selector = vtkSmartPointer<vtkHardwareSelector>::New();
    selector->SetRenderer(Renderer);

    // set rectangular boundary of lasso
    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int maxY = std::numeric_limits<int>::min();

    for (const auto& point : LassoPoints) {
        minX = std::min(minX, point[0]);
        minY = std::min(minY, point[1]);
        maxX = std::max(maxX, point[0]);
        maxY = std::max(maxY, point[1]);
    }
    selector->SetArea(minX, minY, maxX, maxY);

	// set cell selection mode
    selector->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_CELLS);

	// select cells inside the lasso boundary
    vtkSmartPointer<vtkSelection> selection = selector->Select();

	// check if any cells were selected by hardware selector
    if (!selection || selection->GetNumberOfNodes() == 0) {
        wxMessageBox("No cells selected.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    vtkSmartPointer<vtkSelectionNode> selectionNode = selection->GetNode(0);
    
	// check if node contains on cell indices
    vtkSmartPointer<vtkIdTypeArray> selectedIds =
        vtkIdTypeArray::SafeDownCast(selectionNode->GetSelectionList());
    
    if (!selectedIds) {
        wxMessageBox("No cell indices found in selection.", "Error", wxOK | wxICON_ERROR);
        return;
    }

	// cycle through selected cell indices
    for (vtkIdType i = 0; i < selectedIds->GetNumberOfTuples(); ++i) {
        vtkIdType cellId = selectedIds->GetValue(i);

        // get bounds of the cell
        double bounds[6];
        Mesh->GetCellBounds(cellId, bounds);

        // calculate centroid
        double center[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

        // convert world centroid to display point
        double displayCoord[3];
        Renderer->SetWorldPoint(center[0], center[1], center[2], 1.0);
        Renderer->WorldToDisplay();
        Renderer->GetDisplayPoint(displayCoord);

		// check if the cell cetroid is inside the lasso polygon
        if (IsPointInPolygon(displayCoord[0], displayCoord[1], LassoPoints)) {
            SelectedCells->InsertUniqueId(cellId);
			CellColors->SetTuple3(cellId, 255, 0, 0); // highlight selected cell
        }
    }

    // update scene
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