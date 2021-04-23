#include <QString>
#include <QByteArray>

QByteArray suffix(const QString& fileName)
{
	int index;
	if(-1 == (index = fileName.lastIndexOf('.')))
	{
		return QByteArray();
	}
	return QByteArray().append(fileName.right(fileName.size()-index-1).toLower());
}
