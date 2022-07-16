#include <BRepPrimAPI_MakeBox.hxx>

#include <BRep_Tool.hxx>

#include <BRepAlgoAPI_Fuse.hxx>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>

#include <BRepLib.hxx>

#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>

#include <Geom2d_Ellipse.hxx>
#include <Geom2d_TrimmedCurve.hxx>

#include <TopExp_Explorer.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>

#include <TopTools_ListOfShape.hxx>


#include <vtkRenderWindow.h>
#include <vtkNew.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAutoInit.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>

#include <IVtkTools_ShapeDataSource.hxx>



VTK_MODULE_INIT(vtkRenderingOpenGL)
VTK_MODULE_INIT(vtkInteractionStyle)

TopoDS_Shape MakeBottle(const Standard_Real theWidth,
    const Standard_Real theHeight,
    const Standard_Real theThickness)
{
    // Profile : Define Support Points  先创建点
    gp_Pnt aPnt1(-theWidth / 2., 0, 0);
    gp_Pnt aPnt2(-theWidth / 2., -theThickness / 4., 0);
    gp_Pnt aPnt3(0, -theThickness / 2., 0);
    gp_Pnt aPnt4(theWidth / 2., -theThickness / 4., 0);
    gp_Pnt aPnt5(theWidth / 2., 0, 0);

    // Profile : Define the Geometry
    Handle(Geom_TrimmedCurve) anArcOfCircle = GC_MakeArcOfCircle(aPnt2, aPnt3, aPnt4);  // 创建圆弧
    Handle(Geom_TrimmedCurve) aSegment1 = GC_MakeSegment(aPnt1, aPnt2);                        // 创建直线
    Handle(Geom_TrimmedCurve) aSegment2 = GC_MakeSegment(aPnt4, aPnt5);

    // Profile : Define the Topology
    TopoDS_Edge anEdge1 = BRepBuilderAPI_MakeEdge(aSegment1);       // 通过线创建边
    TopoDS_Edge anEdge2 = BRepBuilderAPI_MakeEdge(anArcOfCircle);
    TopoDS_Edge anEdge3 = BRepBuilderAPI_MakeEdge(aSegment2);
    TopoDS_Wire aWire = BRepBuilderAPI_MakeWire(anEdge1, anEdge2, anEdge3); // 将边变成轮廓框（wire）

    // Complete Profile
    gp_Ax1 xAxis = gp::OX();   // X轴
    gp_Trsf aTrsf;                      // 这个类可进行平移、旋转、缩放 镜像 组合操作

    aTrsf.SetMirror(xAxis);
    BRepBuilderAPI_Transform aBRepTrsf(aWire, aTrsf);
    TopoDS_Shape aMirroredShape = aBRepTrsf.Shape();  // 镜像成新Shape
    TopoDS_Wire aMirroredWire = TopoDS::Wire(aMirroredShape);

    BRepBuilderAPI_MakeWire mkWire;  // 将两个Wire合并在一起
    mkWire.Add(aWire);
    mkWire.Add(aMirroredWire);
    TopoDS_Wire myWireProfile = mkWire.Wire();

    // Body : Prism the Profile 
    TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(myWireProfile);  // 根据wire创建底面
    gp_Vec aPrismVec(0, 0, theHeight);
    TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec); // 将面拉伸成体


    // Body : Apply Fillets
    BRepFilletAPI_MakeFillet mkFillet(myBody);
    TopExp_Explorer anEdgeExplorer(myBody, TopAbs_EDGE);
    // 遍历所有边进行倒角操作
    while (anEdgeExplorer.More())
    {
        TopoDS_Edge anEdge = TopoDS::Edge(anEdgeExplorer.Current());
        //Add edge to fillet algorithm
        mkFillet.Add(theThickness / 12., anEdge);
        anEdgeExplorer.Next();
    }
    
    myBody = mkFillet.Shape(); // 获取倒角之后的body
   
    // Body : Add the Neck
    gp_Pnt neckLocation(0, 0, theHeight);
    gp_Dir neckAxis = gp::DZ(); // Z轴矢量
    gp_Ax2 neckAx2(neckLocation, neckAxis);

    Standard_Real myNeckRadius = theThickness / 4.;
    Standard_Real myNeckHeight = theHeight / 10.;

    BRepPrimAPI_MakeCylinder MKCylinder(neckAx2, myNeckRadius, myNeckHeight); // 创建了一个圆柱
    TopoDS_Shape myNeck = MKCylinder.Shape();

    myBody = BRepAlgoAPI_Fuse(myBody, myNeck);  // 将物体组装起来

    // Body : Create a Hollowed Solid
    TopoDS_Face   faceToRemove;
    Standard_Real zMax = -1;

    for (TopExp_Explorer aFaceExplorer(myBody, TopAbs_FACE); aFaceExplorer.More(); aFaceExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(aFaceExplorer.Current()); // 当前的Topo面
        // Check if <aFace> is the top face of the bottle neck
        Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace); // 将Topo face创建Geom_Surface
        // 找到要删除的面，Z坐标最大，同时是Geom_Plane类型
        cout << aSurface->DynamicType() << endl;
        if (aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane))
        {
            Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
            gp_Pnt aPnt = aPlane->Location();
            Standard_Real aZ = aPnt.Z();
            if (aZ > zMax)
            {
                zMax = aZ;
                faceToRemove = aFace;
            }
        }
    }

    TopTools_ListOfShape facesToRemove;
    facesToRemove.Append(faceToRemove);
    BRepOffsetAPI_MakeThickSolid aSolidMaker;
    // 创建空心bottle，需给出删除的面，壁面的厚度，允许误差
    aSolidMaker.MakeThickSolidByJoin(myBody, facesToRemove, -theThickness / 50, 1.e-3);
    myBody = aSolidMaker.Shape();

    // Threading : Create Surfaces
    Handle(Geom_CylindricalSurface) aCyl1 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 0.99);
    Handle(Geom_CylindricalSurface) aCyl2 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 1.05);

    // Threading : Define 2D Curves
    gp_Pnt2d aPnt(2. * M_PI, myNeckHeight / 2.);
    gp_Dir2d aDir(2. * M_PI, myNeckHeight / 4.);
    gp_Ax2d anAx2d(aPnt, aDir);

    Standard_Real aMajor = 2. * M_PI;
    Standard_Real aMinor = myNeckHeight / 10;

    Handle(Geom2d_Ellipse) anEllipse1 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor);
    Handle(Geom2d_Ellipse) anEllipse2 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor / 4);
    Handle(Geom2d_TrimmedCurve) anArc1 = new Geom2d_TrimmedCurve(anEllipse1, 0, M_PI);
    Handle(Geom2d_TrimmedCurve) anArc2 = new Geom2d_TrimmedCurve(anEllipse2, 0, M_PI);
    gp_Pnt2d anEllipsePnt1 = anEllipse1->Value(0);
    gp_Pnt2d anEllipsePnt2 = anEllipse1->Value(M_PI);

    Handle(Geom2d_TrimmedCurve) aSegment = GCE2d_MakeSegment(anEllipsePnt1, anEllipsePnt2);
    // Threading : Build Edges and Wires
    TopoDS_Edge anEdge1OnSurf1 = BRepBuilderAPI_MakeEdge(anArc1, aCyl1);
    TopoDS_Edge anEdge2OnSurf1 = BRepBuilderAPI_MakeEdge(aSegment, aCyl1);
    TopoDS_Edge anEdge1OnSurf2 = BRepBuilderAPI_MakeEdge(anArc2, aCyl2);
    TopoDS_Edge anEdge2OnSurf2 = BRepBuilderAPI_MakeEdge(aSegment, aCyl2);
    TopoDS_Wire threadingWire1 = BRepBuilderAPI_MakeWire(anEdge1OnSurf1, anEdge2OnSurf1);
    TopoDS_Wire threadingWire2 = BRepBuilderAPI_MakeWire(anEdge1OnSurf2, anEdge2OnSurf2);
    BRepLib::BuildCurves3d(threadingWire1);
    BRepLib::BuildCurves3d(threadingWire2);

    // Create Threading
    BRepOffsetAPI_ThruSections aTool(Standard_True);
    aTool.AddWire(threadingWire1);
    aTool.AddWire(threadingWire2);
    aTool.CheckCompatibility(Standard_False);

    TopoDS_Shape myThreading = aTool.Shape();

    // Building the Resulting Compound
    TopoDS_Compound aRes;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound(aRes);
    aBuilder.Add(aRes, myBody);
    aBuilder.Add(aRes, myThreading);

    return aRes;
}



int main()
{

	const TopoDS_Shape& shape = MakeBottle(50, 70, 30);

	vtkNew<vtkRenderWindow>  renwin;
	vtkNew<vtkRenderer> ren;
	renwin->AddRenderer(ren.Get());

	vtkNew<vtkInteractorStyleTrackballCamera> istyle;
	vtkNew<vtkRenderWindowInteractor> iren;

	iren->SetRenderWindow(renwin.Get());
	iren->SetInteractorStyle(istyle.Get());

	vtkNew<IVtkTools_ShapeDataSource> occsource;
	occsource->SetShape(new IVtkOCC_Shape( shape));

	vtkNew<vtkPolyDataMapper> mapper;

	mapper->SetInputConnection(occsource->GetOutputPort());

	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper.Get());
	ren->AddActor(actor.Get());

	renwin->Render();
	iren->Start();

	return 0;
} 