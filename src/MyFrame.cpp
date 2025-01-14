#include "MyFrame.h"
#include "wx/wx.h"

enum
{
    Quit = 1,
    About,
    Load_STL,
    Reset_View,
    Toggle_Edges_And_Picking,
    Save_SubMesh,
    Clear_Selection,
    Lasso_Selection 
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Quit, MyFrame::OnQuit)
EVT_MENU(About, MyFrame::OnAbout)
EVT_MENU(Load_STL, MyFrame::OnLoadSTL)
EVT_MENU(Reset_View, MyFrame::OnResetView)
EVT_MENU(Toggle_Edges_And_Picking, MyFrame::OnToggleEdgesAndPicking)
EVT_MENU(Save_SubMesh, MyFrame::OnSaveSubMesh)
EVT_MENU(Clear_Selection, MyFrame::OnClearSelection)
EVT_MENU(Lasso_Selection, MyFrame::OnLassoSelection)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, -1, title, pos, size)
{
    SetIcon(wxICON(mondrian));

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(Load_STL, _T("&Load mesh\tCtrl-L"), _T("Load an STL file"));
    menuFile->Append(Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

    wxMenu* menuView = new wxMenu;
    menuView->Append(Reset_View, _T("&Reset view\tR"), _T("Reset camera to initial pos"));

    wxMenu* menuMesh = new wxMenu;
    menuMesh->Append(Toggle_Edges_And_Picking, _T("&Toggle Edges and Picking\tCtrl-E"), _T("Toggle edge visibility and enable picking"));
    menuMesh->Append(Clear_Selection, _T("&Clear Selection\tCtrl-C"), _T("Clear selected cells"));
    menuMesh->Append(Save_SubMesh, _T("&Save Sub-Mesh\tCtrl-S"), _T("Save selected sub-mesh"));
    menuMesh->Append(Lasso_Selection, _T("&Lasso Selection\tL"), _T("Enable lasso selection mode")); 

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(About, _T("&About...\tCtrl-A"), _T("Show about dialog"));

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(menuFile, _T("&File"));
    menuBar->Append(menuView, _T("&View"));
    menuBar->Append(menuMesh, _T("&Mesh"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);

    m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW);
    m_pVTKWindow->UseCaptureMouseOn();

    ConstructVTK();
    ConfigureVTK();
    EnablePicking();
}

MyFrame::~MyFrame()
{
    if (m_pVTKWindow) m_pVTKWindow->Delete();
}

void MyFrame::ConstructVTK()
{
    initialCameraPosition[0] = 0.0;
    initialCameraPosition[1] = 0.0;
    initialCameraPosition[2] = 50.0;
    initialCameraFocalPoint[0] = 0.0;
    initialCameraFocalPoint[1] = 0.0;
    initialCameraFocalPoint[2] = 0.0;

    pRenderer = vtkRenderer::New();
    stlActor = vtkSmartPointer<vtkActor>::New();
    cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->SetTolerance(0.0005);
    selectedCells = vtkSmartPointer<vtkIdList>::New();
}

void MyFrame::ConfigureVTK()
{
    pRenderer->GetActiveCamera()->SetPosition(initialCameraPosition);
    pRenderer->GetActiveCamera()->SetFocalPoint(initialCameraFocalPoint);
    pRenderer->GetActiveCamera()->SetViewUp(0.0, 1.0, 0.0);

    pRenderWindow = m_pVTKWindow->GetRenderWindow();
    pRenderWindow->AddRenderer(pRenderer);

    pRenderer->GradientBackgroundOn();
    pRenderer->SetBackground(1, 1, 1);
    pRenderer->SetBackground2(0.5, 0.5, 1.0);
    pRenderer->GetActiveCamera()->Elevation(30.0);
    pRenderer->GetActiveCamera()->Azimuth(30.0);
    pRenderer->GetActiveCamera()->Zoom(1.0);
    pRenderer->GetActiveCamera()->SetClippingRange(1, 1000);
}

void MyFrame::EnablePicking()
{
    auto interactor = m_pVTKWindow->GetRenderWindow()->GetInteractor();

    vtkSmartPointer<MyInteractorStyle> style = vtkSmartPointer<MyInteractorStyle>::New();
    style->SetPicker(cellPicker);
    style->SetRenderer(pRenderer);
    style->SetMesh(originalMesh);
    style->SetCellColors(cellColors);
    style->SetSelectedCells(selectedCells);

    interactor->SetInteractorStyle(style);
}

void MyFrame::OnToggleEdgesAndPicking(wxCommandEvent& WXUNUSED(event))
{
    auto interactor = m_pVTKWindow->GetRenderWindow()->GetInteractor();

    if (edgesVisible) {
        stlActor->GetProperty()->EdgeVisibilityOff();
        interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    }
    else {
        stlActor->GetProperty()->EdgeVisibilityOn();
        stlActor->GetProperty()->SetEdgeColor(0.0, 1.0, 0.0);
        stlActor->GetProperty()->SetLineWidth(2.0);
        EnablePicking();
    }

    edgesVisible = !edgesVisible;
    m_pVTKWindow->Render();
}

void MyFrame::OnClearSelection(wxCommandEvent& WXUNUSED(event))
{
    if (!originalMesh || !cellColors || selectedCells->GetNumberOfIds() == 0) {
        wxMessageBox("No mesh loaded or no cells selected!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    for (vtkIdType i = 0; i < originalMesh->GetNumberOfCells(); ++i) {
        cellColors->SetTuple3(i, 255, 255, 255);
    }
    originalMesh->GetCellData()->SetScalars(cellColors);
    originalMesh->Modified();
    selectedCells->Reset();

    auto interactor = m_pVTKWindow->GetRenderWindow()->GetInteractor();
    interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    stlActor->GetProperty()->EdgeVisibilityOff();

    m_pVTKWindow->Render();
    wxMessageBox("Selection cleared.", "Info", wxOK | wxICON_INFORMATION);

}

void MyFrame::OnSaveSubMesh(wxCommandEvent& WXUNUSED(event))
{
    if (!originalMesh || selectedCells->GetNumberOfIds() == 0) {
        wxMessageBox("No mesh loaded or no cells selected!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    vtkSmartPointer<vtkExtractCells> extractCells = vtkSmartPointer<vtkExtractCells>::New();
    extractCells->SetInputData(originalMesh);
    extractCells->SetCellList(selectedCells);
    extractCells->Update();

    vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputConnection(extractCells->GetOutputPort());
    geometryFilter->Update();

    vtkSmartPointer<vtkPolyData> subMesh = geometryFilter->GetOutput();

    int triangleCount = 0;
    for (vtkIdType i = 0; i < subMesh->GetNumberOfCells(); i++) {
        vtkCell* cell = subMesh->GetCell(i);
        if (cell->GetCellType() == VTK_TRIANGLE) {
            triangleCount++;
        }
    }

    wxFileDialog saveFileDialog(this, _T("Save Sub-Mesh"), "", "", _T("3D files (*.stl;*.obj)|*.stl;*.obj"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString path = saveFileDialog.GetPath();
    std::string filePath = path.mb_str().data();

    if (path.EndsWith(".stl")) {
        vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
        writer->SetFileName(filePath.c_str());
        writer->SetInputData(subMesh);
        writer->Write();
    }
    else if (path.Lower().EndsWith(".obj")) {
        std::ofstream objFile(filePath);
        if (!objFile.is_open()) {
            wxMessageBox("Failed to open file for writing!", "Error", wxOK | wxICON_ERROR);
            return;
        }

        // Write vertices
        vtkPoints* points = subMesh->GetPoints();
        for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
            double coords[3];
            points->GetPoint(i, coords);
            objFile << "v " << coords[0] << " " << coords[1] << " " << coords[2] << "\n";
        }

        // Write normals 
        vtkDataArray* normals = subMesh->GetPointData()->GetNormals();
        if (normals) {
            for (vtkIdType i = 0; i < normals->GetNumberOfTuples(); ++i) {
                double normal[3];
                normals->GetTuple(i, normal);
                objFile << "vn " << normal[0] << " " << normal[1] << " " << normal[2] << "\n";
            }
        }

        // Write faces
        vtkCellArray* cells = subMesh->GetPolys();
        cells->InitTraversal();
        vtkIdType npts;
        vtkIdType* pts;
        while (cells->GetNextCell(npts, pts)) {
            objFile << "f";
            for (vtkIdType i = 0; i < npts; ++i) {
                objFile << " " << (pts[i] + 1); // OBJ: starts from 1!!
                if (normals) {
                    objFile << "//" << (pts[i] + 1); // OBJ: starts from 1!!
                }
            }
            objFile << "\n";
        }

        objFile.close();
    }
    else {
        wxMessageBox("Unsupported file format!", "Error", wxOK | wxICON_ERROR);
    }

    wxString msg;
    msg.Printf(_T("Saving finished, number of mesh elements =  %d"), triangleCount);
    wxMessageBox(msg, "Success", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf(_T("Software Development Task\n"));
    wxMessageBox(msg, _T("About wx-vtk"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnLoadSTL(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog openFileDialog(this, _T("Open 3D file"), "", "", _T("3D files (*.stl;*.obj)|*.stl;*.obj"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString path = openFileDialog.GetPath();

    vtkSmartPointer<vtkPolyData> polyData;

    if (path.Lower().EndsWith(".stl")) {
        vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
        stlReader->SetFileName(path.mb_str().data());
        stlReader->Update();
        polyData = stlReader->GetOutput();
    }
    else if (path.Lower().EndsWith(".obj")) {
        vtkSmartPointer<vtkOBJReader> objReader = vtkSmartPointer<vtkOBJReader>::New();
        objReader->SetFileName(path.mb_str().data());
        objReader->Update();
        polyData = objReader->GetOutput();
    }
    else {
        wxMessageBox("Unsupported file format!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    stlActor->SetMapper(mapper);
    originalMesh = polyData;

    cellColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    cellColors->SetNumberOfComponents(3);

    for (vtkIdType i = 0; i < originalMesh->GetNumberOfCells(); ++i) {
        cellColors->InsertNextTuple3(255, 255, 255);
    }
    originalMesh->GetCellData()->SetScalars(cellColors);

    pRenderer->RemoveAllViewProps();

    double bounds[6];
    stlActor->GetBounds(bounds);
    double centerX = (bounds[0] + bounds[1]) / 2.0;
    double centerY = (bounds[2] + bounds[3]) / 2.0;
    double centerZ = (bounds[4] + bounds[5]) / 2.0;
    pRenderer->GetActiveCamera()->SetFocalPoint(centerX, centerY, centerZ);
    pRenderer->GetActiveCamera()->SetPosition(centerX, centerY, centerZ + (bounds[5] - bounds[4]) * 2.0);
    pRenderer->GetActiveCamera()->SetViewUp(0.0, 1.0, 0.0);
    pRenderer->ResetCamera();

    pRenderer->AddActor(stlActor);
    pRenderer->ResetCamera();

    auto interactor = m_pVTKWindow->GetRenderWindow()->GetInteractor();
    interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    stlActor->GetProperty()->EdgeVisibilityOff();
    m_pVTKWindow->Render();
}

void MyFrame::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    pRenderer->GetActiveCamera()->SetPosition(initialCameraPosition);
    pRenderer->GetActiveCamera()->SetFocalPoint(initialCameraFocalPoint);
    pRenderer->GetActiveCamera()->SetViewUp(0.0, 1.0, 0.0);
    pRenderer->ResetCamera();
    m_pVTKWindow->Render();
}

void MyFrame::OnLassoSelection(wxCommandEvent& WXUNUSED(event))
{
    auto interactor = m_pVTKWindow->GetRenderWindow()->GetInteractor();

    if (interactor->GetInteractorStyle() == nullptr ||
        !MyLassoInteractorStyle::SafeDownCast(interactor->GetInteractorStyle()))
    {
		// if current style is not MyLassoInteractorStyle, switch to it
        vtkSmartPointer<MyLassoInteractorStyle> style = vtkSmartPointer<MyLassoInteractorStyle>::New();
        style->SetPicker(cellPicker);
        style->SetRenderer(pRenderer);
        style->SetMesh(originalMesh);
        style->SetCellColors(cellColors);
        style->SetSelectedCells(selectedCells);
        interactor->SetInteractorStyle(style);

        stlActor->GetProperty()->EdgeVisibilityOn();
        stlActor->GetProperty()->SetEdgeColor(0.0, 1.0, 0.0);
        stlActor->GetProperty()->SetLineWidth(2.0);
    }
    else
    {
		// if current style is MyLassoInteractorStyle, switch to default style
        interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
        stlActor->GetProperty()->EdgeVisibilityOff();

    }

    m_pVTKWindow->Render();
}
