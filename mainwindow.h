#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "controlador.h"
#include <QFileDialog>
#include <QMainWindow>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0);
	~MainWindow();

public slots:
	void imprimir_estado(const QString& estado);
	void aumenta_reloj(const QString& estado);

private slots:
	void seleccionar_archivos();
	void empezar_simulacion();

private:
	Ui::MainWindow* ui;
	QStringList m_rutas_archivos;
	Controlador* m_controlador;
};
#endif // MAINWINDOW_H
