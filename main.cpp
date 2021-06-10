#include <iostream>
#include <casmutils/xtal/structure.hpp>
#include <casmutils/xtal/structure_tools.hpp>
#include <CLI/CLI.hpp>
#include <vector>
#include <math.h>

const double pi = acos(-1);

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
    std::vector<double> input_rotation_axis;
    double input_rotation_angle;
    std::string printflag = "n";
 
    app.add_option("-s,--structure",input_struc_path,"Path to input structure (vasp format).")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output",output_struc_path,"Path to write transformed structure to. Defaults to printing to screen if no output path indicated");
    auto* matrix_opt = app.add_option("-m,--matrix",input_mat_path,"Path to file containing 3x3 transformation matrix.")->check(CLI::ExistingFile);
    auto* rotation_opt = app.add_option("-r,--axis",input_rotation_axis,"Rotation axis in vector form")->expected(3);
    auto* rotation_ang_opt = app.add_option("-a,--angle",input_rotation_angle,"Rotation angle in degrees");
    //reads first character and prints if its anything but an 'n' or 'N'
    app.add_option("-p,--print", printflag,"Prints output structure to screen? y or n, defaults to n")->ignore_case();
 

    rotation_opt->needs(rotation_ang_opt)->excludes(matrix_opt);
    rotation_ang_opt->needs(rotation_opt)->excludes(matrix_opt);
    matrix_opt->excludes(rotation_opt)->excludes(rotation_ang_opt);
    
    CLI11_PARSE(app, argc, argv);

    // Read a structure
    auto input_struc=cu::xtal::Structure::from_poscar(input_struc_path);

    //Checks that a transformation is given, otherwise throws a warning message and quits
    if(!((rotation_opt->count())||matrix_opt->count())) {
        std::cout << "WARNING: No transformation specified\n";
    }

    //Executes rotation (-r -a)
    if(rotation_opt->count()) {
        //Read rotation inputs and normalize the axis
        Eigen::Vector3d rotation_axis;
        for(int i = 0; i<3; i++) {
            rotation_axis(i)=input_rotation_axis[i];
        }
        
        rotation_axis.normalize();

        //create rotation_matrix (in proper format) - check syntax for Eigen 3d matrix
        Eigen::AngleAxisd rotation_AngleAxis(input_rotation_angle*pi/180,rotation_axis);
        Eigen::Matrix3d rotation_matrix = rotation_AngleAxis.matrix();
        
        //Apply rotation to lattice
        auto final_struc=cu::xtal::apply_deformation(input_struc, rotation_matrix);
        
        // Output transformed structure
        if(!(output_struc_path.empty())) {
            cu::xtal::write_poscar(final_struc,output_struc_path);
            if(tolower(printflag[0]) != 'n') {
                std::cout << "printing to screen:\n";
                cu::xtal::print_poscar(final_struc,std::cout);
            }
        }
        //to screen if no output path and print flag is given
        else {
            std::cout << "No output path specified\n";
            if(tolower(printflag[0]) != 'n') {
                std::cout << "printing to screen:\n";
                cu::xtal::print_poscar(final_struc,std::cout);
            }
        }
        
    }

    //Executes transformation (-m)
    else {
    // Read a transformation matrix
    Eigen::Matrix3d transform_mat=read_matrix(input_mat_path);

    // Apply transformation to lattice
    auto final_struc=cu::xtal::apply_deformation(input_struc, transform_mat);
    
    // Output transformed structure
    if(!(output_struc_path.empty())) {
        cu::xtal::write_poscar(final_struc,output_struc_path);
        if(tolower(printflag[0]) != 'n') {
                std::cout << "printing to screen:\n";
                cu::xtal::print_poscar(final_struc,std::cout);
            }
        }
    //to screen if no output path (& print is specifiec)
    else {
            std::cout << "No output path specified\n";
            if(tolower(printflag[0]) != 'n') {
                std::cout << "printing to screen:\n";
                cu::xtal::print_poscar(final_struc,std::cout);
            }
        }
    }
    return 0;
}