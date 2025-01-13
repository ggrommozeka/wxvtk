#ifndef MYFRAME_H
#define MYFRAME_H

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/frame.h>
#include <wxVTKRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkCellPicker.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkUnsignedCharArray.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkSTLWriter.h>
#include <vtkGeometryFilter.h>
#include <vtkExtractCells.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkOBJReader.h>
#include <vtkPointData.h>

#include "MyInteractorStyle.h"
#include "MyLassoInteractorStyle.h"


#define MY_FRAME      101
#define MY_VTK_WINDOW 102

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame();

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnLoadSTL(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    void OnToggleEdgesAndPicking(wxCommandEvent& event);
    void OnSaveSubMesh(wxCommandEvent& event);
    void OnClearSelection(wxCommandEvent& event);
    void OnLassoSelection(wxCommandEvent& event);
protected:
    void ConstructVTK();
    void ConfigureVTK();
    void EnablePicking();

private:
    wxVTKRenderWindowInteractor* m_pVTKWindow;
    vtkSmartPointer<vtkRenderer> pRenderer;
    vtkSmartPointer<vtkRenderWindow> pRenderWindow;
    vtkSmartPointer<vtkActor> stlActor;
    vtkSmartPointer<vtkCellPicker> cellPicker;
    vtkSmartPointer<vtkPolyData> originalMesh;
    vtkSmartPointer<vtkIdList> selectedCells;
    vtkSmartPointer<vtkUnsignedCharArray> cellColors;
    double initialCameraPosition[3];
    double initialCameraFocalPoint[3];
    bool edgesVisible = false;

private:
    DECLARE_EVENT_TABLE()
};

#endif // MYFRAME_H