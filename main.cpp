#include <iostream>
#include <casmutils/xtal/structure.hpp>
#include <casmutils/xtal/structure_tools.hpp>
#include <CLI/CLI.hpp>
#include <vector>

Eigen::Matrix3d read_matrix(const std::filesystem::path& input_path)
{
    std::ifstream mat_stream(input_path);

    Eigen::Matrix3d mat;
    for(int i=0; i<3; ++i)
    {
        for(int j=0; j<3; ++j)
        {
            mat_stream>>mat(i,j);
        }
    }

    mat_stream.close();
    return mat;
}

int main(int argc, char* argv[])
{
    namespace cu=casmutils;

    // Parse CLI
    CLI::App app;

    // Path to input structure
    cu::fs::path input_struc_path;
    cu::fs::path input_mat_path;
    cu::fs::path output_struc_path;
    std::vector<double> rotation_axis_input;
    double rotation_angle_input=0;



    app.add_option("-s,--structure",input_struc_path,"Path to input structure (vasp format).")->required()->check(CLI::ExistingFile);
    app.add_option("-m,--matrix",input_mat_path,"Path to file containing 3x3 transformation matrix.")->check(CLI::ExistingFile);
    //add conditional requiring -a if -r (from CLI readme)
    auto* rotation_opt = app.add_option("-a,--rotation-axis",rotation_axis_input,"Rotation axis in vector form")->expected(3);
    app.add_option("-r,--rotation-angle",rotation_angle_input,"Rotation angle in radians");
    app.add_option("-o,--output",output_struc_path,"Path to write transformed structure to.")->required();

    CLI11_PARSE(app, argc, argv);

    // Read a structre
    auto input_struc=cu::xtal::Structure::from_poscar(input_struc_path);

    if(rotation_opt->count()) {
    //Read rotation inputs and normalize the axis
    double j = rotation_axis_input[0];
    double k = rotation_axis_input[1];
    double l = rotation_axis_input[2];
    Eigen::Vector3d rotation_axis(j,k,l);
    rotation_axis.normalize();

        //create rotation_matrix (in proper format) - check syntax for Eigen 3d matrix
    Eigen::AngleAxisd rotation_AngleAxis(rotation_angle_input,rotation_axis);
    Eigen::Matrix3d rotation_matrix = rotation_AngleAxis.matrix();
    
    //Apply rotation to lattice
    auto final_struc=cu::xtal::apply_deformation(input_struc, rotation_matrix);
    
    // Output transformed structure
    cu::xtal::write_poscar(final_struc,output_struc_path);
    }

    else {
    // Read a transformation matrix (col or rows?)
    Eigen::Matrix3d transform_mat=read_matrix(input_mat_path);

    // Apply transformation to lattice
    auto final_struc=cu::xtal::apply_deformation(input_struc, transform_mat);
    
    // Output transformed structure
    cu::xtal::write_poscar(final_struc,output_struc_path);
    }

    return 0;
}